#include "MainWindow.h"
#include "ComsolParser.h"
#include "PDFExporter.h"
#include "AppTheme.h"
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QProgressDialog>
#include <QFrame>
#include <QDir>
#include <QScreen>
#include <QStyle>

// ============================================================
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("MuonSimViewer  v4.0  |  CCTVal / KIT");
    setMinimumSize(1280, 720);

    if (QScreen* scr = QGuiApplication::primaryScreen()) {
        QRect g = scr->availableGeometry();
        resize(qMin(1600, g.width()-40), qMin(900, g.height()-40));
        move((g.width()-width())/2, (g.height()-height())/2);
    }

    buildMenuBar();
    buildCentralWidget();

    statusBar()->setStyleSheet("QStatusBar { font-size: 10px; }");
    setStatus("Open a COMSOL 3D particle tracking CSV to begin.");

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(40);
    connect(m_animTimer, &QTimer::timeout, this, &MainWindow::onAnimTick);
}

MainWindow::~MainWindow() {
    if (m_videoThread) { m_videoThread->quit(); m_videoThread->wait(); }
}

// ============================================================
void MainWindow::buildMenuBar()
{
    QMenuBar* mb = menuBar();
    QMenu* file = mb->addMenu("&File");
    file->addAction("Open COMSOL CSV...", this, &MainWindow::onOpenFile, QKeySequence::Open);
    file->addSeparator();
    file->addAction("Export PDF Report...",    this, &MainWindow::onExportPDF,   QKeySequence("Ctrl+P"));
    file->addAction("Export Video (MP4)...",   this, &MainWindow::onExportVideo, QKeySequence("Ctrl+M"));
    file->addAction("Export Statistics CSV...",this, &MainWindow::onExportCSV,   QKeySequence("Ctrl+E"));
    file->addSeparator();
    file->addAction("Quit", qApp, &QApplication::quit, QKeySequence::Quit);

    QMenu* view = mb->addMenu("&View");
    view->addAction("Reset Cameras", this, &MainWindow::onResetCameras, QKeySequence("Ctrl+R"));

    QMenu* help = mb->addMenu("&Help");
    help->addAction("About MuonSimViewer...", this, &MainWindow::onAbout);
}

// ============================================================
void MainWindow::buildCentralWidget()
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* vl = new QVBoxLayout(central);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);
    setCentralWidget(central);

    // Main splitter
    QSplitter* hSplit = new QSplitter(Qt::Horizontal, central);
    hSplit->setHandleWidth(3);
    vl->addWidget(hSplit, 1);

    // Left: two 3D viewports stacked
    QSplitter* vSplit = new QSplitter(Qt::Vertical, hSplit);
    vSplit->setHandleWidth(3);

    auto wrapVp = [&](Viewport3D* vp, const QString& title) -> QWidget* {
        QWidget* w = new QWidget();
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0); l->setSpacing(0);
        QWidget* hdr = new QWidget(); hdr->setFixedHeight(26);
        hdr->setStyleSheet(QString("background:%1;border-bottom:1px solid %2;")
                           .arg(Theme::BG_PANEL).arg(Theme::BORDER));
        QHBoxLayout* hl2 = new QHBoxLayout(hdr);
        hl2->setContentsMargins(10,0,10,0);
        QLabel* lbl = new QLabel(title, hdr);
        lbl->setStyleSheet(QString("color:%1;font-size:10px;font-weight:600;letter-spacing:1.5px;").arg(Theme::CYAN));
        hl2->addWidget(lbl); hl2->addStretch();
        l->addWidget(hdr); l->addWidget(vp, 1);
        return w;
    };

    m_beamVp = new Viewport3D(ViewMode::BEAM);
    m_trajVp = new Viewport3D(ViewMode::TRAJECTORIES);
    vSplit->addWidget(wrapVp(m_beamVp, "⬡  BEAM VIEW  ·  Real-time particle positions"));
    vSplit->addWidget(wrapVp(m_trajVp, "⬡  TRAJECTORY VIEW  ·  Colored by initial radius R₀"));
    vSplit->setSizes({400,400});

    // Right: stats panel
    m_stats = new StatsPanel(hSplit);
    m_stats->setMinimumWidth(300);
    m_stats->setMaximumWidth(500);

    hSplit->addWidget(vSplit);
    hSplit->addWidget(m_stats);
    hSplit->setSizes({950, 380});

    // Control bar at bottom
    vl->addWidget(buildControlBar());
}

QWidget* MainWindow::makeSeparator()
{
    QFrame* s = new QFrame();
    s->setFrameShape(QFrame::VLine);
    s->setFixedWidth(1);
    s->setStyleSheet(QString("background:%1;").arg(Theme::BORDER));
    return s;
}

QWidget* MainWindow::buildControlBar()
{
    QWidget* bar = new QWidget();
    bar->setFixedHeight(70);
    bar->setStyleSheet(QString("background:%1;border-top:1px solid %2;")
                       .arg(Theme::BG_PANEL).arg(Theme::BORDER));

    QHBoxLayout* hl = new QHBoxLayout(bar);
    hl->setContentsMargins(12,8,12,8);
    hl->setSpacing(8);

    // Open
    QPushButton* btnOpen = new QPushButton("📂  OPEN CSV");
    btnOpen->setObjectName("btnPrimary");
    btnOpen->setFixedWidth(120);
    btnOpen->setToolTip("Open COMSOL 3D particle tracking CSV");
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    hl->addWidget(btnOpen);
    hl->addWidget(makeSeparator());

    // Play
    m_btnPlay = new QPushButton("▶  PLAY");
    m_btnPlay->setObjectName("btnPrimary");
    m_btnPlay->setFixedWidth(100);
    m_btnPlay->setEnabled(false);
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayToggle);
    hl->addWidget(m_btnPlay);

    // Slider group
    QWidget* sg = new QWidget();
    QVBoxLayout* svl = new QVBoxLayout(sg);
    svl->setContentsMargins(0,0,0,0); svl->setSpacing(2);

    QWidget* slRow = new QWidget();
    QHBoxLayout* shl = new QHBoxLayout(slRow);
    shl->setContentsMargins(0,0,0,0); shl->setSpacing(6);

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0,0);
    m_slider->setEnabled(false);
    m_slider->setToolTip("Drag to scrub through time");
    connect(m_slider, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
    shl->addWidget(m_slider, 1);

    m_lblFrame = new QLabel("—");
    m_lblFrame->setStyleSheet(QString("color:%1;font-family:'Courier New';font-size:11px;min-width:70px;").arg(Theme::TEXT_DIM));
    shl->addWidget(m_lblFrame);
    svl->addWidget(slRow);

    m_lblTime = new QLabel("Time: —");
    m_lblTime->setStyleSheet(QString("color:%1;font-family:'Courier New';font-size:13px;font-weight:600;").arg(Theme::CYAN));
    svl->addWidget(m_lblTime);
    hl->addWidget(sg, 1);

    hl->addWidget(makeSeparator());

    // Export buttons
    m_btnPDF = new QPushButton("📄  PDF");
    m_btnPDF->setObjectName("btnGold");
    m_btnPDF->setFixedWidth(88);
    m_btnPDF->setEnabled(false);
    m_btnPDF->setToolTip("Export PDF analysis report");
    connect(m_btnPDF, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    hl->addWidget(m_btnPDF);

    m_btnVid = new QPushButton("🎬  VIDEO");
    m_btnVid->setObjectName("btnPurple");
    m_btnVid->setFixedWidth(90);
    m_btnVid->setEnabled(false);
    m_btnVid->setToolTip("Export animated MP4 video");
    connect(m_btnVid, &QPushButton::clicked, this, &MainWindow::onExportVideo);
    hl->addWidget(m_btnVid);

    m_btnCSV = new QPushButton("💾  CSV");
    m_btnCSV->setFixedWidth(80);
    m_btnCSV->setEnabled(false);
    m_btnCSV->setToolTip("Export statistics as CSV");
    connect(m_btnCSV, &QPushButton::clicked, this, &MainWindow::onExportCSV);
    hl->addWidget(m_btnCSV);

    hl->addWidget(makeSeparator());

    m_btnReset = new QPushButton("↺  RESET");
    m_btnReset->setFixedWidth(80);
    m_btnReset->setEnabled(false);
    m_btnReset->setToolTip("Reset 3D cameras");
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::onResetCameras);
    hl->addWidget(m_btnReset);

    // Progress widgets (hidden by default)
    m_progBar = new QProgressBar();
    m_progBar->setRange(0,100);
    m_progBar->setFixedWidth(180);
    m_progBar->hide();
    hl->addWidget(m_progBar);

    m_progMsg = new QLabel();
    m_progMsg->setStyleSheet(QString("color:%1;font-size:10px;").arg(Theme::TEXT_DIM));
    m_progMsg->hide();
    hl->addWidget(m_progMsg);

    return bar;
}

// ============================================================
//  File Loading
// ============================================================
void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open COMSOL 3D Particle Tracking CSV",
        QDir::homePath(), "CSV files (*.csv);;All files (*)");
    if (!path.isEmpty()) loadFile(path);
}

void MainWindow::loadFile(const QString& path)
{
    QProgressDialog prog("Loading COMSOL data...", "Cancel", 0, 100, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setMinimumDuration(0);
    prog.show();

    bool cancelled = false;
    SimulationData data = ComsolParser::parse(path, [&](int pct, const QString& msg) {
        prog.setValue(pct);
        prog.setLabelText(msg);
        QApplication::processEvents();
        if (prog.wasCanceled()) cancelled = true;
    });

    if (cancelled || !data.is_valid()) {
        QMessageBox::critical(this, "Load Error",
            cancelled ? "Loading cancelled."
                      : "Failed to load:\n" + ComsolParser::lastError());
        return;
    }

    m_simData = std::move(data);
    m_dataLoaded = true;
    applyData();
    setStatus(QString("Loaded: %1  |  %2 particles × %3 timesteps  |  Efficiency: %4%")
              .arg(QFileInfo(path).fileName())
              .arg(m_simData.n_particles)
              .arg(m_simData.n_timesteps)
              .arg(m_simData.efficiency_pct, 0, 'f', 1));
}

void MainWindow::applyData()
{
    m_beamVp->setData(&m_simData, 350);
    m_trajVp->setData(&m_simData, 350);
    m_stats->setData(&m_simData);

    m_slider->setRange(0, m_simData.n_timesteps - 1);
    m_slider->setEnabled(true);
    m_btnPlay->setEnabled(true);
    m_btnPDF->setEnabled(true);
    m_btnVid->setEnabled(true);
    m_btnCSV->setEnabled(true);
    m_btnReset->setEnabled(true);

    setFrame(0);
    setWindowTitle(QString("MuonSimViewer v4.0  |  %1  |  %2 μ  |  η=%3%")
                   .arg(QString::fromStdString(
                        m_simData.meta.model_name.empty() ? "COMSOL Simulation"
                                                          : m_simData.meta.model_name))
                   .arg(m_simData.n_particles)
                   .arg(m_simData.efficiency_pct, 0, 'f', 1));
}

void MainWindow::setFrame(int f)
{
    if (!m_dataLoaded) return;
    f = qBound(0, f, m_simData.n_timesteps - 1);
    m_currentFrame = f;
    m_beamVp->setFrame(f);
    m_trajVp->setFrame(f);
    m_stats->updateFrame(f);

    float t_ns = m_simData.time_to_ns(m_simData.timesteps[f]);
    m_lblTime->setText(QString("t = %1 %2")
                       .arg(t_ns, 0, 'f', 3)
                       .arg(QString::fromStdString(m_simData.time_unit())));
    m_lblFrame->setText(QString("f %1 / %2").arg(f).arg(m_simData.n_timesteps-1));

    m_slider->blockSignals(true);
    m_slider->setValue(f);
    m_slider->blockSignals(false);
}

void MainWindow::onSliderChanged(int val) {
    if (m_playing) setPlaying(false);
    setFrame(val);
}

// ============================================================
void MainWindow::onPlayToggle() { setPlaying(!m_playing); }

void MainWindow::setPlaying(bool on)
{
    m_playing = on;
    if (on) {
        m_btnPlay->setText("⏸  PAUSE");
        if (m_currentFrame >= m_simData.n_timesteps - 1) setFrame(0);
        m_animTimer->start();
    } else {
        m_btnPlay->setText("▶  PLAY");
        m_animTimer->stop();
    }
}

void MainWindow::onAnimTick()
{
    if (!m_dataLoaded) { setPlaying(false); return; }
    int next = m_currentFrame + 1;
    if (next >= m_simData.n_timesteps) { setPlaying(false); return; }
    setFrame(next);
}

// ============================================================
void MainWindow::onExportPDF()
{
    if (!m_dataLoaded) return;
    QString path = QFileDialog::getSaveFileName(
        this, "Export PDF Report", "SimulationReport.pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;

    showProgress(true, "Generating PDF...", 0);
    bool ok = PDFExporter::exportReport(path, &m_simData, m_beamVp, m_trajVp, m_currentFrame,
        [this](int pct, const QString& msg) {
            showProgress(true, msg, pct);
            QApplication::processEvents();
        });
    showProgress(false);
    if (ok) setStatus("PDF saved: " + path, 5000);
    else    QMessageBox::critical(this, "Export Error", "Failed to generate PDF.");
}

void MainWindow::onExportVideo()
{
    if (!m_dataLoaded) return;
    QString path = QFileDialog::getSaveFileName(
        this, "Export Video", "MuonSimulation.mp4", "MP4 (*.mp4)");
    if (path.isEmpty()) return;

    if (m_videoThread) { m_videoThread->quit(); m_videoThread->wait(); delete m_videoThread; m_videoThread = nullptr; }
    if (m_videoExporter) { delete m_videoExporter; m_videoExporter = nullptr; }

    m_videoThread   = new QThread(this);
    m_videoExporter = new VideoExporter();
    m_videoExporter->moveToThread(m_videoThread);
    connect(m_videoExporter, &VideoExporter::progress,  this, &MainWindow::onVideoProgress);
    connect(m_videoExporter, &VideoExporter::finished,  this, &MainWindow::onVideoFinished);

    VideoExporter::Settings s;
    s.outputPath = path;
    s.fps = 25; s.width = 1920; s.height = 540;

    m_videoThread->start();
    showProgress(true, "Starting video render...", 0);
    m_btnVid->setEnabled(false);

    QMetaObject::invokeMethod(m_videoExporter, [this, s]() {
        m_videoExporter->render(&m_simData, m_beamVp, m_trajVp, s);
    }, Qt::QueuedConnection);
}

void MainWindow::onVideoProgress(int pct, const QString& msg) {
    showProgress(true, msg, pct);
}
void MainWindow::onVideoFinished(bool ok, const QString& path, const QString& err)
{
    showProgress(false);
    m_btnVid->setEnabled(true);
    if (ok) setStatus("Video saved: " + path, 6000);
    else    QMessageBox::critical(this, "Video Error", err);
}

void MainWindow::onExportCSV()
{
    if (!m_dataLoaded) return;
    QString path = QFileDialog::getSaveFileName(
        this, "Export Statistics CSV", "statistics.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot write: " + path); return;
    }
    QTextStream out(&f);
    out << "frame,time,in_flight,detected,lost,r_rms_cm,sigma_x_cm,sigma_z_cm,efficiency_pct\n";
    for (int i = 0; i < m_simData.n_timesteps; ++i) {
        const FrameStats& s = m_simData.stats[i];
        out << i << "," << m_simData.time_to_ns(s.time) << ","
            << s.in_flight << "," << s.detected << "," << s.lost << ","
            << s.r_rms_cm  << "," << s.sigma_x_cm << "," << s.sigma_z_cm << ","
            << s.efficiency << "\n";
    }
    f.close();
    setStatus("CSV saved: " + path, 4000);
}

void MainWindow::onResetCameras() {
    m_beamVp->resetCamera();
    m_trajVp->resetCamera();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About MuonSimViewer v4.0",
        "<h2 style='color:#00e5ff;'>MuonSimViewer 4.0</h2>"
        "<p><b>CCTVal / KIT Muon Transport Simulation Analyzer</b></p>"
        "<p>High-performance C++/Qt6 application for COMSOL 3D particle tracking.</p><hr>"
        "<b>Features:</b><ul>"
        "<li>Generic COMSOL CSV parser</li>"
        "<li>Dual OpenGL 3D viewports with real-time animation</li>"
        "<li>Interactive time slider (t-parametrized trajectories)</li>"
        "<li>Live particle counters and efficiency tracking</li>"
        "<li>Statistical charts: radial distribution, beam spread vs time</li>"
        "<li>PDF scientific report | MP4 video export | CSV statistics</li>"
        "</ul>");
}

void MainWindow::showProgress(bool show, const QString& msg, int pct) {
    m_progBar->setVisible(show);
    m_progMsg->setVisible(show);
    if (show) { m_progBar->setValue(pct); m_progMsg->setText(msg); }
}

void MainWindow::setStatus(const QString& msg, int ms) {
    ms > 0 ? statusBar()->showMessage(msg, ms) : statusBar()->showMessage(msg);
}
