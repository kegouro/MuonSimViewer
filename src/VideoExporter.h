#pragma once
#include "SimulationData.h"
#include "Viewport3D.h"
#include <QString>
#include <QObject>
#include <functional>

// ============================================================
//  VIDEO EXPORTER — ffmpeg-based MP4 renderer
// ============================================================
class VideoExporter : public QObject {
    Q_OBJECT
public:
    struct Settings {
        int fps = 25;
        int dpi = 130;
        QString outputPath;
        int width = 1920;
        int height = 540;
        QString codec = "libx264";
        int crf = 22;
    };

    explicit VideoExporter(QObject* parent = nullptr);

    // Async render – emits signals
    void render(const SimulationData* data,
                Viewport3D* beamVp,
                Viewport3D* trajVp,
                const Settings& settings);

    void cancel();

signals:
    void progress(int pct, const QString& msg);
    void finished(bool ok, const QString& path, const QString& error);

private:
    bool m_cancel = false;
};
