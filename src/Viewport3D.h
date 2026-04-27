#pragma once
#include "SimulationData.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector3D>
#include <QPoint>
#include <QTimer>
#include <vector>

// ============================================================
//  VIEWPORT3D — OpenGL 3D rendering of tube + trajectories
// ============================================================

enum class ViewMode { BEAM, TRAJECTORIES };

class Viewport3D : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit Viewport3D(ViewMode mode, QWidget* parent = nullptr);
    ~Viewport3D() override;

    void setData(const SimulationData* data, int maxTraj = 350);
    void setFrame(int frame);
    void resetCamera();
    QImage grabFrame();

    ViewMode viewMode() const { return m_mode; }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    // Shaders
    void initShaders();
    void buildGeometry();
    void buildTubeGeometry();
    void buildTrajectoryGeometry();

    // Drawing
    void drawTube();
    void drawTrajectories(int upToFrame);
    void drawActivePoints(int frame);
    void drawGrid();
    void drawHUD(QPainter& p, int frame);

    // Camera
    QMatrix4x4 mvpMatrix() const;
    QVector3D unproject(QPoint p) const;

    // Data
    const SimulationData* m_data = nullptr;
    ViewMode m_mode;
    int m_currentFrame = 0;
    int m_maxTraj = 350;

    // Particle selection for rendering
    std::vector<int> m_selectedIdx;   // particle indices to draw
    std::vector<QVector3D> m_colors;  // color per selected particle
    std::vector<float> m_alphas;

    // Camera state
    float m_azimuth   = 40.0f;
    float m_elevation = 22.0f;
    float m_zoom      = 1.0f;
    QVector3D m_center;
    QPoint m_lastMouse;
    bool m_dragging = false;

    // GL objects
    QOpenGLShaderProgram* m_prog = nullptr;
    QOpenGLShaderProgram* m_flatProg = nullptr;

    // Tube geometry
    std::vector<float> m_tubeVerts;    // surface vertices
    std::vector<float> m_tubeLines;    // wireframe lines

    // Trajectory geometry (all frames pre-baked per particle)
    struct TrajGeom {
        std::vector<float> verts; // xyz per frame
        int stopFrame;
        bool reached;
        QVector3D color;
    };
    std::vector<TrajGeom> m_trajs;

    // Point positions per frame [frame][particle] = xyz
    // We compute on the fly to avoid huge memory
};
