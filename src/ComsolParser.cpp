#include "ComsolParser.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include <limits>
#include <numeric>

QString ComsolParser::s_last_error;

// ============================================================
SimulationData ComsolParser::parse(const QString& filepath, ProgressCb progress)
{
    SimulationData data;
    s_last_error.clear();

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        s_last_error = "Cannot open file: " + filepath;
        return data;
    }

    if (progress) progress(0, "Opening file...");

    QTextStream in(&file);
    int lineNum = 0;
    bool headerDone = false;
    QStringList columnNames;
    int valuesPerTimestep = 5; // qx, qy, qz, pidx, t

    // ── Parse header comments ────────────────────────────────
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNum++;

        if (line.startsWith('%')) {
            QString content = line.mid(1).trimmed();
            if (content.startsWith("Model,"))
                data.meta.model_name = content.mid(6).toStdString();
            else if (content.startsWith("Version,"))
                data.meta.version = content.mid(8).toStdString();
            else if (content.startsWith("Date,"))
                data.meta.date = content.mid(5).remove('"').toStdString();
            else if (content.startsWith("Dimension,"))
                data.meta.dimension = content.mid(10).toInt();
            else if (content.startsWith("Nodes,"))
                data.meta.nodes = content.mid(6).toInt();
            else if (content.startsWith("Expressions,"))
                data.meta.expressions = content.mid(12).toInt();
            else if (content.startsWith("Description,"))
                data.meta.description = content.mid(12).remove('"').toStdString();
            else if (content.startsWith("Índice,") || content.startsWith("Index,")) {
                // Column header line
                columnNames = content.split(',');
                for(const auto& cn : columnNames) data.meta.column_names.push_back(cn.toStdString());
            }
        } else {
            // End of comments – data begins
            headerDone = true;

            // The first data line: particle 1
            // Determine values per timestep from column names
            if (!columnNames.isEmpty()) {
                // Skip first column (index), then group into sets
                // Each timestep has qx, qy, qz, pidx, t
                int numDataCols = columnNames.size() - 1;
                if (numDataCols > 0) {
                    // Find how many unique time values exist
                    // by counting "@ t=" tokens in the header
                    int tCount = 0;
                    for (const auto& cn : columnNames)
                        if (cn.contains("@ t=") && cn.contains("(s)"))
                            tCount++;
                    if (tCount > 0) {
                        valuesPerTimestep = numDataCols / tCount;
                    }
                }
            }

            // Process the first data line
            QStringList parts = line.split(',');
            if (!parts.isEmpty()) {
                Particle p;
                p.id = parts[0].trimmed().toInt();
                QVector<double> vals;
                for (int i = 1; i < parts.size(); ++i)
                    vals.append(parts[i].trimmed().toDouble());

                int nTimesteps = vals.size() / valuesPerTimestep;
                p.frames.reserve(nTimesteps);

                for (int t = 0; t < nTimesteps; ++t) {
                    int base = t * valuesPerTimestep;
                    if (base + valuesPerTimestep - 1 >= vals.size()) break;
                    ParticleFrame f;
                    f.pos.x = static_cast<float>(vals[base + 0]);
                    f.pos.y = static_cast<float>(vals[base + 1]);
                    f.pos.z = static_cast<float>(vals[base + 2]);
                    f.time  = static_cast<float>(vals[base + 4]);
                    p.frames.push_back(f);
                }
                if (!p.frames.empty())
                    data.particles.push_back(std::move(p));
            }
            break;
        }
    }

    if (progress) progress(5, "Parsing particles...");

    // ── Parse data lines ─────────────────────────────────────
    qint64 fileSize = file.size();
    qint64 bytesRead = file.pos();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        bytesRead += line.size() + 2;
        lineNum++;

        if (line.isEmpty() || line.startsWith('%')) continue;

        QStringList parts = line.split(',');
        if (parts.size() < 2) continue;

        Particle p;
        p.id = parts[0].trimmed().toInt();

        QVector<double> vals;
        vals.reserve(parts.size() - 1);
        for (int i = 1; i < parts.size(); ++i)
            vals.append(parts[i].trimmed().toDouble());

        int nTimesteps = vals.size() / valuesPerTimestep;
        p.frames.reserve(nTimesteps);

        for (int t = 0; t < nTimesteps; ++t) {
            int base = t * valuesPerTimestep;
            if (base + valuesPerTimestep - 1 >= vals.size()) break;
            ParticleFrame f;
            f.pos.x = static_cast<float>(vals[base + 0]);
            f.pos.y = static_cast<float>(vals[base + 1]);
            f.pos.z = static_cast<float>(vals[base + 2]);
            f.time  = static_cast<float>(vals[base + 4]);
            p.frames.push_back(f);
        }

        if (!p.frames.empty())
            data.particles.push_back(std::move(p));

        // Progress update every 50 particles
        if (progress && (data.particles.size() % 50 == 0)) {
            int pct = fileSize > 0 ? (int)(bytesRead * 80 / fileSize) : 40;
            progress(5 + std::min(pct, 80),
                     QString("Loaded %1 particles...").arg(data.particles.size()));
        }
    }

    file.close();

    if (data.particles.empty()) {
        s_last_error = "No particle data found in file.";
        return data;
    }

    if (progress) progress(85, "Computing statistics...");

    // ── Extract timestep array from first particle ───────────
    if (!data.particles.empty()) {
        for (const auto& f : data.particles[0].frames)
            data.timesteps.push_back(f.time);
    }

    data.n_particles  = static_cast<int>(data.particles.size());
    data.n_timesteps  = static_cast<int>(data.timesteps.size());

    // ── Compute derived data ─────────────────────────────────
    computeDerivedData(data);

    if (progress) progress(100, "Done.");
    return data;
}

// ============================================================
void ComsolParser::computeDerivedData(SimulationData& data)
{
    if (data.particles.empty()) return;

    // Y axis bounds
    float yMin =  std::numeric_limits<float>::max();
    float yMax = -std::numeric_limits<float>::max();

    for (auto& p : data.particles) {
        if (p.frames.empty()) continue;
        for (const auto& f : p.frames) {
            yMin = std::min(yMin, f.pos.y);
            yMax = std::max(yMax, f.pos.y);
        }
        p.r_initial = p.frames[0].pos.r_transverse();
    }

    data.y_min = yMin;
    data.y_max = yMax;
    data.meta_y = yMin + 0.08f;

    // Time range in ns
    if (!data.timesteps.empty()) {
        data.t_min_ns = data.time_to_ns(data.timesteps.front());
        data.t_max_ns = data.time_to_ns(data.timesteps.back());
    }

    // Auto-detect tube radius from max transverse position
    autoDetectTubeRadius(data);

    // Normalize r_initial
    float rMin = std::numeric_limits<float>::max();
    float rMax = -std::numeric_limits<float>::max();
    for (const auto& p : data.particles) {
        rMin = std::min(rMin, p.r_initial);
        rMax = std::max(rMax, p.r_initial);
    }
    float rRange = std::max(rMax - rMin, 1e-12f);
    for (auto& p : data.particles)
        p.r_norm = (p.r_initial - rMin) / rRange;

    // Classify particles and find stop frames
    classifyParticles(data);
    computeFrameStats(data);

    // Summary
    data.n_detected = 0;
    for (const auto& p : data.particles)
        if (p.reached_detector) ++data.n_detected;
    data.n_lost = data.n_particles - data.n_detected;
    data.efficiency_pct = (float)data.n_detected / data.n_particles * 100.0f;
}

// ============================================================
void ComsolParser::autoDetectTubeRadius(SimulationData& data)
{
    // Estimate tube radius from the 95th percentile of transverse positions
    std::vector<float> r_all;
    r_all.reserve(data.particles.size() * data.n_timesteps);
    for (const auto& p : data.particles)
        for (const auto& f : p.frames)
            r_all.push_back(f.pos.r_transverse());

    if (r_all.empty()) return;

    std::sort(r_all.begin(), r_all.end());
    float r95 = r_all[(int)(r_all.size() * 0.95)];
    float rMax = r_all.back();

    // Round up to nearest 0.05
    float estimated = std::ceil(rMax / 0.05f) * 0.05f;
    data.tube_radius = std::max(estimated, 0.05f);
}

// ============================================================
void ComsolParser::classifyParticles(SimulationData& data)
{
    const float freeze_threshold = 1e-15f;

    for (auto& p : data.particles) {
        if ((int)p.frames.size() < 2) {
            p.stop_frame = (int)p.frames.size() - 1;
            continue;
        }

        p.stop_frame = (int)p.frames.size() - 1;
        for (int f = 1; f < (int)p.frames.size(); ++f) {
            float dy = std::abs(p.frames[f].pos.y - p.frames[f-1].pos.y);
            if (dy < freeze_threshold) {
                p.stop_frame = f;
                break;
            }
        }

        // Check if particle reached detector plane
        p.reached_detector = (p.frames.back().pos.y <= data.meta_y);
        p.hit_wall = !p.reached_detector;
    }
}

// ============================================================
void ComsolParser::computeFrameStats(SimulationData& data)
{
    data.stats.resize(data.n_timesteps);

    for (int f = 0; f < data.n_timesteps; ++f) {
        FrameStats& s = data.stats[f];
        s.time = data.timesteps[f];

        std::vector<float> xs, zs, rs;
        int detected = 0, lost = 0, in_flight = 0;

        for (const auto& p : data.particles) {
            bool is_stopped = (f >= p.stop_frame);
            bool was_det = is_stopped && p.reached_detector;
            bool was_lost = is_stopped && !p.reached_detector;

            if (was_det)       ++detected;
            else if (was_lost) ++lost;
            else {
                ++in_flight;
                const auto& pos = p.frames[std::min(f, (int)p.frames.size()-1)].pos;
                xs.push_back(pos.x);
                zs.push_back(pos.z);
                rs.push_back(pos.r_transverse());
            }
        }

        s.in_flight = in_flight;
        s.detected  = detected;
        s.lost      = lost;
        s.efficiency = (float)detected / data.n_particles * 100.0f;

        if (!rs.empty()) {
            // RMS radius
            float sum_r2 = 0;
            for (float r : rs) sum_r2 += r * r;
            s.r_rms_cm = std::sqrt(sum_r2 / rs.size()) * 100.0f;

            // Sigma X
            float mx = 0;
            for (float x : xs) mx += x;
            mx /= xs.size();
            float vx = 0;
            for (float x : xs) vx += (x-mx)*(x-mx);
            s.sigma_x_cm = std::sqrt(vx / xs.size()) * 100.0f;

            // Sigma Z
            float mz = 0;
            for (float z : zs) mz += z;
            mz /= zs.size();
            float vz = 0;
            for (float z : zs) vz += (z-mz)*(z-mz);
            s.sigma_z_cm = std::sqrt(vz / zs.size()) * 100.0f;
        }
    }
}

// ============================================================
QString ComsolParser::lastError() { return s_last_error; }
