#include "Viewport3D.h"
#include "AppTheme.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QFontMetrics>
#include <cmath>
#include <random>
#include <algorithm>

static constexpr float PI = 3.14159265358979f;

// ============================================================
Viewport3D::Viewport3D(ViewMode mode, QWidget* parent)
    : QOpenGLWidget(parent), m_mode(mode)
{
    setMinimumSize(400, 300);
    QSurfaceFormat fmt;
    fmt.setSamples(4);
    fmt.setDepthBufferSize(24);
    setFormat(fmt);
}

Viewport3D::~Viewport3D() {
    makeCurrent();
    delete m_prog;
    delete m_flatProg;
    doneCurrent();
}

// ============================================================
void Viewport3D::setData(const SimulationData* data, int maxTraj)
{
    m_data = data;
    m_maxTraj = maxTraj;
    m_currentFrame = 0;
    if (data) {
        m_center = QVector3D(0, (data->y_min + data->y_max) * 0.5f, 0);
        buildGeometry();
        if (m_mode == ViewMode::BEAM)      { m_azimuth = 225; m_elevation = 18; }
        else                               { m_azimuth =  40; m_elevation = 22; }
    }
    update();
}

void Viewport3D::setFrame(int f) {
    m_currentFrame = f;
    update();
}

void Viewport3D::resetCamera() {
    if (m_mode == ViewMode::BEAM) { m_azimuth = 225; m_elevation = 18; }
    else                          { m_azimuth =  40; m_elevation = 22; }
    m_zoom = 1.0f;
    update();
}

QImage Viewport3D::grabFrame() {
    return grabFramebuffer();
}

// ============================================================
//  OpenGL setup
// ============================================================
void Viewport3D::initializeGL()
{
    initializeOpenGLFunctions();
    this->glClearColor(0.012f, 0.031f, 0.063f, 1.0f);
    this->glEnable(GL_DEPTH_TEST);
    this->glEnable(GL_BLEND);
    this->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    this->glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    initShaders();
}

void Viewport3D::resizeGL(int w, int h) {
    this->glViewport(0, 0, w, h);
}

void Viewport3D::initShaders()
{
    // Simple vertex-colored shader
    const char* vsSrc = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec4 aColor;
        uniform mat4 uMVP;
        out vec4 vColor;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            vColor = aColor;
            gl_PointSize = 4.0;
        }
    )";
    const char* fsSrc = R"(
        #version 330 core
        in vec4 vColor;
        out vec4 fragColor;
        void main() { fragColor = vColor; }
    )";
    m_prog = new QOpenGLShaderProgram(this);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   vsSrc);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, fsSrc);
    m_prog->link();
}

// ============================================================
//  Camera / MVP
// ============================================================
QMatrix4x4 Viewport3D::mvpMatrix() const
{
    QMatrix4x4 proj, view, model;
    float aspect = (float)width() / std::max(height(), 1);
    proj.perspective(45.0f, aspect, 0.01f, 200.0f);

    float az  = m_azimuth * PI / 180.0f;
    float el  = m_elevation * PI / 180.0f;
    float dist = (m_data ? 1.5f * (m_data->y_max - m_data->y_min) : 5.0f) / m_zoom;

    QVector3D eye(
        m_center.x() + dist * std::cos(el) * std::sin(az),
        m_center.y() + dist * std::sin(el),
        m_center.z() + dist * std::cos(el) * std::cos(az)
    );
    view.lookAt(eye, m_center, QVector3D(0,1,0));
    return proj * view * model;
}

// ============================================================
//  Mouse / Wheel
// ============================================================
void Viewport3D::mousePressEvent(QMouseEvent* e) {
    m_lastMouse = e->pos();
    m_dragging = true;
}
void Viewport3D::mouseMoveEvent(QMouseEvent* e) {
    if (!m_dragging) return;
    QPoint d = e->pos() - m_lastMouse;
    m_azimuth   += d.x() * 0.4f;
    m_elevation  = qBound(-89.0f, m_elevation - d.y() * 0.4f, 89.0f);
    m_lastMouse  = e->pos();
    update();
}
void Viewport3D::wheelEvent(QWheelEvent* e) {
    float delta = e->angleDelta().y() / 120.0f;
    m_zoom = qBound(0.1f, m_zoom * (1.0f + delta * 0.1f), 10.0f);
    update();
}

// ============================================================
//  Geometry building
// ============================================================
void Viewport3D::buildGeometry()
{
    if (!m_data) return;
    buildTubeGeometry();
    buildTrajectoryGeometry();
}

void Viewport3D::buildTubeGeometry()
{
    if (!m_data) return;
    const int N = 80;
    float R = m_data->tube_radius;
    float yMin = m_data->y_min - 0.05f;
    float yMax = m_data->y_max + 0.05f;

    // Surface mesh (2 rings × N points)
    m_tubeVerts.clear();
    for (int i = 0; i <= N; ++i) {
        float t = (float)i / N * 2 * PI;
        float cx = R * std::cos(t), cz = R * std::sin(t);
        // Bottom vertex
        m_tubeVerts.insert(m_tubeVerts.end(), {cx, yMin, cz});
        // Top vertex
        m_tubeVerts.insert(m_tubeVerts.end(), {cx, yMax, cz});
    }

    // Line geometry: generatrices + rings
    m_tubeLines.clear();
    const int NL = 12;
    for (int i = 0; i < NL; ++i) {
        float t = (float)i / NL * 2 * PI;
        float cx = R * std::cos(t), cz = R * std::sin(t);
        m_tubeLines.insert(m_tubeLines.end(), {cx, yMin, cz, cx, yMax, cz});
    }
    // Entry ring (blue)
    for (int i = 0; i <= N; ++i) {
        float t0 = (float)i / N * 2 * PI;
        float t1 = (float)(i+1) / N * 2 * PI;
        m_tubeLines.insert(m_tubeLines.end(), {
            R*cosf(t0), yMax, R*sinf(t0),
            R*cosf(t1), yMax, R*sinf(t1)
        });
    }
    // Detector ring (green)
    for (int i = 0; i <= N; ++i) {
        float t0 = (float)i / N * 2 * PI;
        float t1 = (float)(i+1) / N * 2 * PI;
        m_tubeLines.insert(m_tubeLines.end(), {
            R*cosf(t0), yMin, R*sinf(t0),
            R*cosf(t1), yMin, R*sinf(t1)
        });
    }
}

void Viewport3D::buildTrajectoryGeometry()
{
    if (!m_data) return;
    m_trajs.clear();
    m_selectedIdx.clear();

    // Separate successful and lost particles
    std::vector<int> succIdx, lostIdx;
    for (int i = 0; i < (int)m_data->particles.size(); ++i) {
        if (m_data->particles[i].reached_detector) succIdx.push_back(i);
        else                                       lostIdx.push_back(i);
    }

    // Pick rendering candidates
    std::mt19937 rng(42);
    int nSucc = (int)succIdx.size();
    int nLost = std::min((int)lostIdx.size(), m_maxTraj - nSucc);
    if (nLost > 0 && !lostIdx.empty()) {
        std::shuffle(lostIdx.begin(), lostIdx.end(), rng);
        lostIdx.resize(std::min((int)lostIdx.size(), nLost));
    }

    auto addParticle = [&](int idx) {
        const Particle& p = m_data->particles[idx];
        TrajGeom tg;
        tg.stopFrame = p.stop_frame;
        tg.reached   = p.reached_detector;

        if (m_mode == ViewMode::BEAM) {
            // Bicolor: cyan for success, dim red for lost
            tg.color = p.reached_detector
                ? QVector3D(0.0f, 0.898f, 1.0f)
                : QVector3D(1.0f, 0.251f, 0.376f);
        } else {
            // Plasma gradient by r_initial for success, dim red for lost
            if (p.reached_detector) {
                float t = 0.15f + p.r_norm * 0.75f;
                // Approximate plasma colormap
                tg.color = QVector3D(
                    0.9f * t + 0.1f,
                    0.3f * (1-t) + 0.1f * t,
                    0.8f * (1-t)
                );
            } else {
                tg.color = QVector3D(0.8f, 0.1f, 0.15f);
            }
        }

        tg.verts.reserve(p.frames.size() * 3);
        for (const auto& f : p.frames) {
            tg.verts.push_back(f.pos.x);
            tg.verts.push_back(f.pos.y);
            tg.verts.push_back(f.pos.z);
        }
        m_trajs.push_back(std::move(tg));
        m_selectedIdx.push_back(idx);
    };

    for (int i : succIdx) addParticle(i);
    for (int i : lostIdx) addParticle(i);
}

// ============================================================
//  Rendering
// ============================================================
void Viewport3D::paintGL()
{
    this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!m_data || !m_prog) return;

    QMatrix4x4 mvp = mvpMatrix();
    m_prog->bind();
    m_prog->setUniformValue("uMVP", mvp);

    drawTube();
    drawTrajectories(m_currentFrame);
    drawActivePoints(m_currentFrame);

    m_prog->release();

    // HUD overlay
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawHUD(painter, m_currentFrame);
}

// ── Helper: draw colored line array ─────────────────────────
static void glDrawLines(QOpenGLExtraFunctions* gl,
                        QOpenGLShaderProgram* prog,
                        const std::vector<float>& verts,
                        QVector4D color, float lineWidth = 1.0f)
{
    if (verts.empty()) return;

    // Build interleaved pos+color buffer
    std::vector<float> buf;
    buf.reserve(verts.size() / 3 * 7);
    for (size_t i = 0; i + 2 < verts.size(); i += 3) {
        buf.push_back(verts[i]); buf.push_back(verts[i+1]); buf.push_back(verts[i+2]);
        buf.push_back(color.x()); buf.push_back(color.y());
        buf.push_back(color.z()); buf.push_back(color.w());
    }

    GLuint vao, vbo;
    gl->glGenVertexArrays(1, &vao);
    gl->glGenBuffers(1, &vbo);
    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.data(), GL_STREAM_DRAW);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
    gl->glEnableVertexAttribArray(1);

    gl->glLineWidth(lineWidth);
    gl->glDrawArrays(GL_LINES, 0, (GLsizei)(verts.size() / 3));

    gl->glDeleteBuffers(1, &vbo);
    gl->glDeleteVertexArrays(1, &vao);
}

void Viewport3D::drawTube()
{
    if (!m_data) return;

    // Semi-transparent surface (triangle strip)
    {
        std::vector<float> buf;
        buf.reserve(m_tubeVerts.size() / 3 * 7);
        for (size_t i = 0; i + 2 < m_tubeVerts.size(); i += 3) {
            buf.push_back(m_tubeVerts[i]);
            buf.push_back(m_tubeVerts[i+1]);
            buf.push_back(m_tubeVerts[i+2]);
            // Tube surface: dark blue, very transparent
            buf.push_back(0.06f); buf.push_back(0.18f); buf.push_back(0.33f); buf.push_back(0.07f);
        }
        GLuint vao, vbo;
        this->glGenVertexArrays(1, &vao);
        this->glGenBuffers(1, &vbo);
        this->glBindVertexArray(vao);
        this->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        this->glBufferData(GL_ARRAY_BUFFER, buf.size()*sizeof(float), buf.data(), GL_STREAM_DRAW);
        this->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)0);
        this->glEnableVertexAttribArray(0);
        this->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
        this->glEnableVertexAttribArray(1);
        this->glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)(buf.size()/7));
        this->glDeleteBuffers(1, &vbo);
        this->glDeleteVertexArrays(1, &vao);
    }

    // Wireframe lines
    // Generatrices (blue)
    {
        std::vector<float> genLines;
        const int NL = 12;
        float R = m_data->tube_radius;
        float yMin = m_data->y_min - 0.05f;
        float yMax = m_data->y_max + 0.05f;
        for (int i = 0; i < NL; ++i) {
            float t = (float)i / NL * 2 * PI;
            float cx = R * cosf(t), cz = R * sinf(t);
            genLines.insert(genLines.end(), {cx, yMin, cz, cx, yMax, cz});
        }
        glDrawLines(this, m_prog, genLines, {0.1f, 0.29f, 0.5f, 0.4f}, 0.8f);
    }

    // Entry ring (blue highlight)
    {
        const int N = 80;
        float R = m_data->tube_radius;
        float yMax = m_data->y_max + 0.05f;
        std::vector<float> ringVerts;
        for (int i = 0; i < N; ++i) {
            float t0 = (float)i / N * 2 * PI;
            float t1 = (float)(i+1) / N * 2 * PI;
            ringVerts.insert(ringVerts.end(), {
                R*cosf(t0), yMax, R*sinf(t0),
                R*cosf(t1), yMax, R*sinf(t1)
            });
        }
        glDrawLines(this, m_prog, ringVerts, {0.25f, 0.63f, 0.88f, 0.85f}, 2.0f);
    }

    // Detector ring (green)
    {
        const int N = 80;
        float R = m_data->tube_radius;
        float yMin = m_data->y_min - 0.05f;
        std::vector<float> ringVerts;
        for (int i = 0; i < N; ++i) {
            float t0 = (float)i / N * 2 * PI;
            float t1 = (float)(i+1) / N * 2 * PI;
            ringVerts.insert(ringVerts.end(), {
                R*cosf(t0), yMin, R*sinf(t0),
                R*cosf(t1), yMin, R*sinf(t1)
            });
        }
        glDrawLines(this, m_prog, ringVerts, {0.22f, 1.0f, 0.08f, 0.90f}, 2.2f);
    }

    // Intermediate reference rings
    {
        const int N = 60, NR = 7;
        float R = m_data->tube_radius;
        for (int r = 1; r <= NR; ++r) {
            float yy = m_data->y_min + (m_data->y_max - m_data->y_min) * (float)r / (NR+1);
            std::vector<float> ring;
            for (int i = 0; i < N; ++i) {
                float t0 = (float)i/N*2*PI, t1 = (float)(i+1)/N*2*PI;
                ring.insert(ring.end(), {R*cosf(t0), yy, R*sinf(t0), R*cosf(t1), yy, R*sinf(t1)});
            }
            glDrawLines(this, m_prog, ring, {0.1f, 0.25f, 0.45f, 0.18f}, 0.5f);
        }
    }

    // Detector crosshair
    {
        float d = m_data->tube_radius * 0.15f;
        float yMin = m_data->y_min - 0.05f;
        std::vector<float> cross = {
            -d, yMin, 0,  d, yMin, 0,
             0, yMin, -d, 0, yMin, d
        };
        glDrawLines(this, m_prog, cross, {0.22f, 1.0f, 0.08f, 0.70f}, 1.2f);
    }
}

void Viewport3D::drawTrajectories(int upToFrame)
{
    if (m_trajs.empty()) return;

    for (const auto& tg : m_trajs) {
        int nFrames = (int)(tg.verts.size() / 3);
        int drawFrames = std::min(upToFrame + 1, std::min(tg.stopFrame + 1, nFrames));
        if (drawFrames < 2) continue;

        bool isSuccess = tg.reached;
        float alpha = isSuccess ? (m_mode == ViewMode::BEAM ? 1.0f : 0.85f) : 0.12f;
        float lw    = isSuccess ? 1.6f : 0.45f;

        // Build line strip buffer
        std::vector<float> buf;
        buf.reserve(drawFrames * 7);
        for (int f = 0; f < drawFrames; ++f) {
            buf.push_back(tg.verts[f*3+0]);
            buf.push_back(tg.verts[f*3+1]);
            buf.push_back(tg.verts[f*3+2]);
            buf.push_back(tg.color.x());
            buf.push_back(tg.color.y());
            buf.push_back(tg.color.z());
            buf.push_back(alpha);
        }

        GLuint vao, vbo;
        this->glGenVertexArrays(1, &vao);
        this->glGenBuffers(1, &vbo);
        this->glBindVertexArray(vao);
        this->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        this->glBufferData(GL_ARRAY_BUFFER, buf.size()*sizeof(float), buf.data(), GL_STREAM_DRAW);
        this->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)0);
        this->glEnableVertexAttribArray(0);
        this->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
        this->glEnableVertexAttribArray(1);
        ::glLineWidth(lw);
        this->glDrawArrays(GL_LINE_STRIP, 0, drawFrames);
        this->glDeleteBuffers(1, &vbo);
        this->glDeleteVertexArrays(1, &vao);
    }
}

void Viewport3D::drawActivePoints(int frame)
{
    if (!m_data || m_trajs.empty()) return;
    if (frame < 0 || frame >= m_data->n_timesteps) return;

    // Collect active particle positions
    std::vector<float> pts;
    for (size_t k = 0; k < m_trajs.size(); ++k) {
        const auto& tg = m_trajs[k];
        int pidx = m_selectedIdx[k];
        const Particle& p = m_data->particles[pidx];
        if (frame < p.stop_frame && frame < (int)p.frames.size()) {
            const auto& pos = p.frames[frame].pos;
            pts.push_back(pos.x); pts.push_back(pos.y); pts.push_back(pos.z);
            pts.push_back(1.0f); pts.push_back(1.0f); pts.push_back(1.0f); pts.push_back(0.8f);
        }
    }
    if (pts.empty()) return;

    GLuint vao, vbo;
    this->glGenVertexArrays(1, &vao);
    this->glGenBuffers(1, &vbo);
    this->glBindVertexArray(vao);
    this->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    this->glBufferData(GL_ARRAY_BUFFER, pts.size()*sizeof(float), pts.data(), GL_STREAM_DRAW);
    this->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)0);
    this->glEnableVertexAttribArray(0);
    this->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
    this->glEnableVertexAttribArray(1);
    ::glEnable(GL_PROGRAM_POINT_SIZE);
    ::glPointSize(3.5f);
    this->glDrawArrays(GL_POINTS, 0, (GLsizei)(pts.size()/7));
    this->glDeleteBuffers(1, &vbo);
    this->glDeleteVertexArrays(1, &vao);
}

void Viewport3D::drawHUD(QPainter& painter, int frame)
{
    if (!m_data) return;

    const QString title = (m_mode == ViewMode::BEAM)
        ? "BEAM VIEW — Current Positions"
        : "TRAJECTORY VIEW — Colored by R₀";

    // Title
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.setPen(QColor(Theme::CYAN));
    painter.drawText(10, 20, title);

    // Frame info
    if (frame < m_data->n_timesteps) {
        const FrameStats& s = m_data->stats[frame];
        float t_ns = m_data->time_to_ns(s.time);

        QString info = QString("t = %1 %2   |   In-flight: %3   |   Detected: %4   |   Eff: %5%")
            .arg(t_ns, 0, 'f', 2)
            .arg(QString::fromStdString(m_data->time_unit()))
            .arg(s.in_flight)
            .arg(s.detected)
            .arg(s.efficiency, 0, 'f', 1);

        painter.setFont(QFont("Courier New", 8));
        painter.setPen(QColor(Theme::TEXT_DIM));
        painter.drawText(10, height() - 10, info);
    }

    // Legend
    if (m_mode == ViewMode::BEAM) {
        int lx = width() - 140, ly = 20;
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor(Theme::CYAN));
        painter.drawText(lx + 14, ly, "Successful μ");
        painter.fillRect(lx, ly-9, 10, 10, QColor(Theme::CYAN));
        ly += 16;
        painter.setPen(QColor(Theme::RED));
        painter.drawText(lx + 14, ly, "Lost μ");
        painter.fillRect(lx, ly-9, 10, 10, QColor(Theme::RED));
        ly += 16;
        painter.setPen(QColor(Theme::GREEN));
        painter.drawText(lx + 14, ly, "Detector");
        painter.fillRect(lx, ly-9, 10, 10, QColor(Theme::GREEN));
    }
}
