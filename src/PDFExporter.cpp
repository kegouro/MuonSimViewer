#include "PDFExporter.h"
#include "AppTheme.h"
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QFont>
#include <QColor>
#include <QRect>
#include <QDateTime>
#include <cmath>

static void drawRect(QPainter& p, QRect r, QColor fill, QColor border = Qt::transparent) {
    p.fillRect(r, fill);
    if (border != Qt::transparent) {
        p.setPen(QPen(border, 1));
        p.drawRect(r);
    }
}

static void drawText(QPainter& p, QRect r, const QString& txt,
                     QColor col, int fontSize=10, bool bold=false,
                     int align = Qt::AlignLeft | Qt::AlignVCenter)
{
    QFont f("Courier New");
    f.setPointSize(fontSize);
    f.setBold(bold);
    p.setFont(f);
    p.setPen(col);
    p.drawText(r, align, txt);
}

bool PDFExporter::exportReport(const QString& filepath,
                                const SimulationData* data,
                                Viewport3D* beam,
                                Viewport3D* traj,
                                int currentFrame,
                                ProgressCb cb)
{
    if (!data) return false;

    QPdfWriter writer(filepath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setResolution(120);
    writer.setTitle("Muon Transport Simulation — CCTVal/KIT Report");
    writer.setCreator("MuonSimViewer v4.0");

    QPainter p(&writer);
    if (!p.isActive()) return false;

    QRect page = p.viewport();
    int W = page.width(), H = page.height();
    int margin = 40;
    int inner_W = W - 2 * margin;

    // ─── Color palette
    QColor bg     (3, 8, 16);
    QColor panel  (11, 21, 35);
    QColor cyan   (0, 229, 255);
    QColor green  (57, 255, 20);
    QColor red    (255, 64, 96);
    QColor gold   (255, 211, 42);
    QColor textP  (192, 216, 240);
    QColor textD  (80, 112, 160);
    QColor border (28, 61, 99);

    if (cb) cb(5, "Rendering background...");

    // ─── Background
    p.fillRect(page, bg);

    // ─── Header bar
    QRect hdr(0, 0, W, 52);
    p.fillRect(hdr, QColor(11, 21, 35));
    p.setPen(QPen(border, 1));
    p.drawLine(0, 52, W, 52);

    // Logo / title
    {
        QFont f("Courier New"); f.setPointSize(14); f.setBold(true);
        p.setFont(f); p.setPen(cyan);
        p.drawText(QRect(margin, 10, 500, 30), Qt::AlignLeft | Qt::AlignVCenter,
                   "MUON TRANSPORT SIMULATION — ANALYSIS REPORT");
    }
    {
        QFont f("Courier New"); f.setPointSize(8);
        p.setFont(f); p.setPen(textD);
        p.drawText(QRect(W - 260, 10, 220, 14), Qt::AlignRight,
                   QString("Generated: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm")));
        p.drawText(QRect(W - 260, 26, 220, 14), Qt::AlignRight,
                   "MuonSimViewer v4.0 | CCTVal / KIT");
        if (!data->meta.model_name.empty()) {
            p.drawText(QRect(W - 260, 40, 220, 14), Qt::AlignRight,
                       QString("Model: %1").arg(QString::fromStdString(data->meta.model_name)));
        }
    }

    int y = 62;

    // ─── Summary cards row
    if (cb) cb(15, "Drawing summary cards...");
    {
        struct Card { QString label, value; QColor accent; };
        std::vector<Card> cards = {
            {"INJECTED PARTICLES",  QString::number(data->n_particles),        QColor(Theme::BLUE_FIELD)},
            {"DETECTED",            QString::number(data->n_detected),          green},
            {"LOST (WALL)",         QString::number(data->n_lost),              red},
            {"FINAL EFFICIENCY",    QString("%1%").arg(data->efficiency_pct,0,'f',1), gold},
            {"TUBE RADIUS",         QString("%1 m").arg(data->tube_radius,0,'f',3), textP},
            {"TIME RANGE",          QString("%1–%2 %3")
                                       .arg(data->t_min_ns,0,'f',1)
                                       .arg(data->t_max_ns,0,'f',1)
                                       .arg(QString::fromStdString(data->time_unit())), textP},
        };
        int cw = inner_W / (int)cards.size() - 6;
        int cx = margin;
        for (const auto& c : cards) {
            QRect cr(cx, y, cw, 52);
            p.fillRect(cr, panel);
            p.setPen(QPen(border));
            p.drawRoundedRect(cr, 4, 4);

            QFont lf("Courier New"); lf.setPointSize(7); lf.setBold(false);
            p.setFont(lf); p.setPen(textD);
            p.drawText(QRect(cx+6, y+4, cw-8, 16), Qt::AlignLeft | Qt::AlignVCenter, c.label);

            QFont vf("Courier New"); vf.setPointSize(13); vf.setBold(true);
            p.setFont(vf); p.setPen(c.accent);
            p.drawText(QRect(cx+6, y+20, cw-8, 28), Qt::AlignLeft | Qt::AlignVCenter, c.value);

            cx += cw + 6;
        }
        y += 62;
    }

    // ─── 3D viewport screenshots
    if (cb) cb(30, "Capturing 3D views...");
    if (beam && traj) {
        QImage beamImg = beam->grabFrame();
        QImage trajImg = traj->grabFrame();

        int vpH = (H - y - margin - 120) / 2;
        int vpW = (inner_W - 8) / 2;
        QRect beamR(margin, y, vpW, vpH);
        QRect trajR(margin + vpW + 8, y, vpW, vpH);

        // Draw images with border
        auto drawVp = [&](QRect r, const QImage& img, const QString& title) {
            p.fillRect(r, panel);
            p.setPen(QPen(border));
            p.drawRoundedRect(r, 4, 4);
            if (!img.isNull()) {
                p.drawImage(r.adjusted(2,2,-2,-2), img);
            }
            QFont f("Courier New"); f.setPointSize(8); f.setBold(true);
            p.setFont(f); p.setPen(cyan);
            p.drawText(QRect(r.x()+8, r.y()+6, r.width()-16, 18),
                       Qt::AlignLeft | Qt::AlignVCenter, title);
        };
        drawVp(beamR, beamImg, "▣  BEAM VIEW — Current Positions");
        drawVp(trajR, trajImg, "▣  INDIVIDUAL TRAJECTORIES — Colored by R₀");

        y += vpH + 10;
    }

    // ─── Frame statistics at current frame
    if (cb) cb(55, "Drawing frame statistics...");
    if (currentFrame < data->n_timesteps) {
        const FrameStats& fs = data->stats[currentFrame];
        float t_ns = data->time_to_ns(fs.time);

        struct KV { QString k, v; QColor col; };
        std::vector<KV> kvs = {
            {"Current Frame", QString::number(currentFrame), textP},
            {"Time", QString("%1 %2").arg(t_ns,0,'f',3).arg(QString::fromStdString(data->time_unit())), cyan},
            {"In-flight", QString::number(fs.in_flight), cyan},
            {"Detected", QString::number(fs.detected), green},
            {"Lost", QString::number(fs.lost), red},
            {"Efficiency", QString("%1%").arg(fs.efficiency,0,'f',2), gold},
            {"r_rms", QString("%1 cm").arg(fs.r_rms_cm,0,'f',3), gold},
            {"σ_x", QString("%1 cm").arg(fs.sigma_x_cm,0,'f',3), textP},
            {"σ_z", QString("%1 cm").arg(fs.sigma_z_cm,0,'f',3), textP},
        };
        int kw = inner_W / (int)kvs.size() - 4;
        int kx = margin;
        int ch = 40;
        for (const auto& kv : kvs) {
            QRect cr(kx, y, kw, ch);
            p.fillRect(cr, panel);
            p.setPen(QPen(border));
            p.drawRoundedRect(cr, 3, 3);

            QFont lf("Courier New"); lf.setPointSize(7);
            p.setFont(lf); p.setPen(textD);
            p.drawText(QRect(kx+4, y+3, kw-6, 14), Qt::AlignLeft, kv.k.toUpper());

            QFont vf("Courier New"); vf.setPointSize(10); vf.setBold(true);
            p.setFont(vf); p.setPen(kv.col);
            p.drawText(QRect(kx+4, y+18, kw-6, 18), Qt::AlignLeft, kv.v);
            kx += kw + 4;
        }
        y += ch + 10;
    }

    // ─── Frame statistics table (first 15 rows)
    if (cb) cb(70, "Generating statistics table...");
    {
        struct Col { QString hdr; int w; };
        std::vector<Col> cols = {
            {"Frame", 50}, {"Time", 70}, {"In-Flight", 70},
            {"Detected", 70}, {"Lost", 60}, {"r_rms [cm]", 80},
            {"σ_x [cm]", 80}, {"σ_z [cm]", 80}, {"η [%]", 70}
        };
        int th = 18;  // row height
        int tableW = 0; for (const auto& c : cols) tableW += c.w;
        int tx = margin, ty = y;

        // Header
        int cx2 = tx;
        p.fillRect(QRect(tx, ty, tableW, th), QColor(16, 29, 48));
        for (const auto& col : cols) {
            p.setPen(QPen(border));
            p.drawRect(QRect(cx2, ty, col.w, th));
            QFont f("Courier New"); f.setPointSize(7); f.setBold(true);
            p.setFont(f); p.setPen(textD);
            p.drawText(QRect(cx2+3, ty, col.w-4, th), Qt::AlignLeft|Qt::AlignVCenter, col.hdr);
            cx2 += col.w;
        }
        ty += th;

        int rowsLeft = (H - ty - margin) / th;
        int nRows = std::min(rowsLeft - 2, data->n_timesteps);
        for (int f = 0; f < nRows; ++f) {
            const FrameStats& s = data->stats[f];
            bool isCurrentFrame = (f == currentFrame);
            QColor rowBg = isCurrentFrame ? QColor(0, 80, 100) : (f%2==0 ? panel : QColor(16,26,42));
            cx2 = tx;
            p.fillRect(QRect(tx, ty, tableW, th), rowBg);
            QStringList vals = {
                QString::number(f),
                QString("%1").arg(data->time_to_ns(s.time),0,'f',3),
                QString::number(s.in_flight),
                QString::number(s.detected),
                QString::number(s.lost),
                QString("%1").arg(s.r_rms_cm,0,'f',3),
                QString("%1").arg(s.sigma_x_cm,0,'f',3),
                QString("%1").arg(s.sigma_z_cm,0,'f',3),
                QString("%1").arg(s.efficiency,0,'f',2)
            };
            for (int i = 0; i < (int)cols.size(); ++i) {
                p.setPen(QPen(border));
                p.drawRect(QRect(cx2, ty, cols[i].w, th));
                QFont f2("Courier New"); f2.setPointSize(7);
                p.setFont(f2);
                p.setPen(isCurrentFrame ? cyan : textP);
                p.drawText(QRect(cx2+3, ty, cols[i].w-4, th), Qt::AlignLeft|Qt::AlignVCenter, vals[i]);
                cx2 += cols[i].w;
            }
            ty += th;
        }
    }

    // ─── Footer
    p.setPen(QPen(border, 1));
    p.drawLine(0, H - 20, W, H - 20);
    QFont f2("Courier New"); f2.setPointSize(7);
    p.setFont(f2); p.setPen(textD);
    p.drawText(QRect(margin, H - 18, W - 2*margin, 16), Qt::AlignCenter,
               QString("CCTVal / KIT Muon Beamline Analysis  |  %1 particles  |  "
                        "Efficiency: %2%  |  Tube R: %3 m")
               .arg(data->n_particles)
               .arg(data->efficiency_pct, 0, 'f', 1)
               .arg(data->tube_radius, 0, 'f', 3));

    p.end();
    if (cb) cb(100, "PDF saved.");
    return true;
}
