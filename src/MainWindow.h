#pragma once
#include "SimulationData.h"
#include "Viewport3D.h"
#include "StatsPanel.h"
#include "VideoExporter.h"
#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QProgressBar>
#include <QThread>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    void loadFile(const QString& path);

private slots:
    void onOpenFile();
    void onSliderChanged(int val);
    void onPlayToggle();
    void onAnimTick();
    void onExportPDF();
    void onExportVideo();
    void onExportCSV();
    void onResetCameras();
    void onAbout();
    void onVideoProgress(int pct, const QString& msg);
    void onVideoFinished(bool ok, const QString& path, const QString& err);

private:
    void buildMenuBar();
    void buildCentralWidget();
    QWidget* buildControlBar();
    QWidget* makeSeparator();
    void applyData();
    void setFrame(int f);
    void setPlaying(bool on);
    void showProgress(bool show, const QString& msg = "", int pct = 0);
    void setStatus(const QString& msg, int timeout_ms = 0);

    SimulationData m_simData;
    bool m_dataLoaded = false;

    Viewport3D*   m_beamVp   = nullptr;
    Viewport3D*   m_trajVp   = nullptr;
    StatsPanel*   m_stats    = nullptr;
    QSlider*      m_slider   = nullptr;
    QLabel*       m_lblTime  = nullptr;
    QLabel*       m_lblFrame = nullptr;
    QPushButton*  m_btnPlay  = nullptr;
    QPushButton*  m_btnPDF   = nullptr;
    QPushButton*  m_btnVid   = nullptr;
    QPushButton*  m_btnCSV   = nullptr;
    QPushButton*  m_btnReset = nullptr;
    QProgressBar* m_progBar  = nullptr;
    QLabel*       m_progMsg  = nullptr;

    QTimer*        m_animTimer    = nullptr;
    bool           m_playing      = false;
    int            m_currentFrame = 0;
    VideoExporter* m_videoExporter = nullptr;
    QThread*       m_videoThread   = nullptr;
};
