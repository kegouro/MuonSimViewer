#include "VideoExporter.h"
#include <QProcess>
#include <QImage>
#include <QThread>
#include <QDir>
#include <QTemporaryDir>
#include <QDebug>
#include <QPainter>

VideoExporter::VideoExporter(QObject* parent) : QObject(parent) {}

void VideoExporter::cancel() { m_cancel = true; }

void VideoExporter::render(const SimulationData* data,
                            Viewport3D* beamVp,
                            Viewport3D* trajVp,
                            const Settings& settings)
{
    m_cancel = false;
    if (!data) {
        emit finished(false, "", "No data loaded.");
        return;
    }

    // Check ffmpeg availability
    QProcess check;
    check.start("ffmpeg", {"-version"});
    check.waitForFinished(3000);
    if (check.exitCode() != 0 || check.state() == QProcess::NotRunning) {
        // Try to find ffmpeg
        QString ffmpegPath;
        QStringList searchPaths = {"/usr/bin/ffmpeg", "/usr/local/bin/ffmpeg", "ffmpeg"};
        for (const auto& path : searchPaths) {
            QProcess test;
            test.start(path, {"-version"});
            test.waitForFinished(2000);
            if (test.exitCode() == 0) { ffmpegPath = path; break; }
        }
        if (ffmpegPath.isEmpty()) {
            emit finished(false, "", "ffmpeg not found. Please install ffmpeg.");
            return;
        }
    }

    emit progress(2, "Creating temporary frames...");

    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
        emit finished(false, "", "Cannot create temp directory.");
        return;
    }

    int frameW = settings.width / 2;
    int frameH = settings.height;
    int n = data->n_timesteps;

    for (int f = 0; f < n && !m_cancel; ++f) {
        // Update both viewports to this frame
        beamVp->setFrame(f);
        trajVp->setFrame(f);

        // Grab frames from OpenGL
        beamVp->update();
        trajVp->update();

        QImage beam = beamVp->grabFrame();
        QImage traj = trajVp->grabFrame();

        // Composite side-by-side
        QImage composite(settings.width, settings.height, QImage::Format_RGB32);
        composite.fill(QColor(3, 8, 16));

        QPainter painter(&composite);
        // Scale and draw each half
        if (!beam.isNull())
            painter.drawImage(QRect(0, 0, frameW, frameH), beam);
        if (!traj.isNull())
            painter.drawImage(QRect(frameW, 0, frameW, frameH), traj);

        // Time overlay
        if (f < (int)data->stats.size()) {
            float t_ns = data->time_to_ns(data->stats[f].time);
            painter.setFont(QFont("Courier New", 10, QFont::Bold));
            painter.setPen(QColor(0, 229, 255));
            painter.drawText(10, 20,
                QString("t = %1 %2  |  η = %3%  |  Detected: %4/%5")
                .arg(t_ns, 0, 'f', 2)
                .arg(QString::fromStdString(data->time_unit()))
                .arg(data->stats[f].efficiency, 0, 'f', 1)
                .arg(data->stats[f].detected)
                .arg(data->n_particles));
        }
        // Divider line
        painter.setPen(QPen(QColor(28, 61, 99), 1));
        painter.drawLine(frameW, 0, frameW, frameH);
        painter.end();

        QString framePath = QString("%1/frame_%2.png")
            .arg(tmpDir.path())
            .arg(f, 6, 10, QChar('0'));
        composite.save(framePath);

        int pct = 2 + (int)((float)f / n * 80);
        emit progress(pct, QString("Rendering frame %1/%2").arg(f+1).arg(n));
    }

    if (m_cancel) {
        emit finished(false, "", "Cancelled by user.");
        return;
    }

    // Run ffmpeg
    emit progress(83, "Encoding video...");
    QString inputPattern = tmpDir.path() + "/frame_%06d.png";
    QStringList args = {
        "-y",
        "-framerate", QString::number(settings.fps),
        "-i", inputPattern,
        "-vcodec", settings.codec,
        "-crf", QString::number(settings.crf),
        "-pix_fmt", "yuv420p",
        "-movflags", "+faststart",
        settings.outputPath
    };

    QProcess ffmpeg;
    ffmpeg.start("ffmpeg", args);
    if (!ffmpeg.waitForStarted(5000)) {
        emit finished(false, "", "Failed to start ffmpeg.");
        return;
    }
    while (!ffmpeg.waitForFinished(200)) {
        QThread::msleep(100);
        if (m_cancel) { ffmpeg.kill(); emit finished(false,"","Cancelled."); return; }
        emit progress(85, "Encoding...");
    }

    if (ffmpeg.exitCode() != 0) {
        QString err = ffmpeg.readAllStandardError();
        emit finished(false, "", "ffmpeg error: " + err.left(200));
        return;
    }

    emit progress(100, "Video exported.");
    emit finished(true, settings.outputPath, "");
}
