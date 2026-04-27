#include "StatsPanel.h"
#include "AppTheme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QHeaderView>
#include <cmath>

// ─── Helper: stat card ──────────────────────────────────────
static QWidget* makeStatCard(const QString& key, QLabel** valLabel,
                              const QString& accent = Theme::CYAN)
{
    QFrame* card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setObjectName("panel");
    card->setFixedHeight(68);
    card->setStyleSheet(QString(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 6px; }")
        .arg(Theme::BG_WIDGET)
        .arg(Theme::BORDER));

    QVBoxLayout* vl = new QVBoxLayout(card);
    vl->setContentsMargins(10,6,10,6);
    vl->setSpacing(2);

    QLabel* kl = new QLabel(key, card);
    kl->setObjectName("labelKey");
    kl->setStyleSheet(QString("color: %1; font-size: 10px; letter-spacing: 1px;").arg(Theme::TEXT_DIM));

    *valLabel = new QLabel("—", card);
    (*valLabel)->setObjectName("labelValue");
    (*valLabel)->setStyleSheet(QString(
        "color: %1; font-family: 'Courier New'; font-size: 16px; font-weight: 700;")
        .arg(accent));

    vl->addWidget(kl);
    vl->addWidget(*valLabel);
    return card;
}

// ─── Helper: section title ──────────────────────────────────
static QLabel* makeSectionTitle(const QString& text) {
    QLabel* l = new QLabel(text);
    l->setStyleSheet(
        "color: #5070a0; font-size: 10px; font-weight: 600; "
        "letter-spacing: 2px; padding: 12px 0 4px 0;");
    return l;
}

// ============================================================
StatsPanel::StatsPanel(QWidget* parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_content = new QWidget(this);
    setWidget(m_content);
    buildUI();
}

// ============================================================
void StatsPanel::buildUI()
{
    QVBoxLayout* main = new QVBoxLayout(m_content);
    main->setContentsMargins(12, 12, 12, 24);
    main->setSpacing(0);

    // ── Title
    QLabel* title = new QLabel("SIMULATION ANALYTICS");
    title->setObjectName("labelTitle");
    title->setStyleSheet(
        "color: #00e5ff; font-size: 14px; font-weight: 700; "
        "letter-spacing: 3px; padding: 4px 0 8px 0;");
    main->addWidget(title);

    // ── Live Counters section
    main->addWidget(makeSectionTitle("▸  LIVE PARTICLE COUNTER"));
    {
        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(6);

        auto addCard = [&](int row, int col, const QString& key, QLabel** lbl,
                           const QString& accent = Theme::CYAN) {
            grid->addWidget(makeStatCard(key, lbl, accent), row, col);
        };

        addCard(0, 0, "IN TRANSIT",    &m_lblInFlight, Theme::CYAN);
        addCard(0, 1, "AT DETECTOR",   &m_lblDetNow,   Theme::GREEN);
        addCard(0, 2, "LOST (WALL)",   &m_lblLostNow,  Theme::RED);
        addCard(1, 0, "EFFICIENCY",    &m_lblEffNow,   Theme::GOLD);
        addCard(1, 1, "r_rms",         &m_lblRmsNow,   Theme::GOLD);
        addCard(1, 2, "TIME",          &m_lblTimeNow,  Theme::TEXT_PRIMARY);

        main->addLayout(grid);
    }

    // ── Final Summary section
    main->addWidget(makeSectionTitle("▸  SIMULATION SUMMARY"));
    {
        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(6);
        auto addCard = [&](int row, int col, const QString& key, QLabel** lbl,
                           const QString& accent = Theme::TEXT_PRIMARY) {
            grid->addWidget(makeStatCard(key, lbl, accent), row, col);
        };
        addCard(0, 0, "TOTAL INJECTED", &m_lblTotal,     Theme::BLUE_FIELD);
        addCard(0, 1, "DETECTED",        &m_lblDetected,  Theme::GREEN);
        addCard(0, 2, "LOST",            &m_lblLost,      Theme::RED);
        addCard(1, 0, "FINAL EFFICIENCY",&m_lblEffic,     Theme::GOLD);
        addCard(1, 1, "TUBE RADIUS",     &m_lblTubeR,     Theme::TEXT_PRIMARY);
        addCard(1, 2, "TIME RANGE",      &m_lblTimeRange, Theme::TEXT_PRIMARY);
        main->addLayout(grid);
    }

    // ── Radial distribution histogram
    main->addWidget(makeSectionTitle("▸  RADIAL DISTRIBUTION — Current Frame"));
    buildHistogramChart();
    main->addWidget(m_histView);

    // ── RMS over time chart
    main->addWidget(makeSectionTitle("▸  BEAM SPREAD vs TIME"));
    buildRmsChart();
    main->addWidget(m_rmsView);

    // ── Per-frame stats table
    main->addWidget(makeSectionTitle("▸  PER-FRAME STATISTICS TABLE"));
    m_frameTable = new QTableWidget();
    m_frameTable->setAlternatingRowColors(true);
    m_frameTable->setMinimumHeight(180);
    main->addWidget(m_frameTable);

    // ── Simulation metadata
    main->addWidget(makeSectionTitle("▸  SIMULATION METADATA"));
    m_metaTable = new QTableWidget();
    m_metaTable->setAlternatingRowColors(true);
    m_metaTable->setMinimumHeight(120);
    main->addWidget(m_metaTable);

    main->addStretch();
}

// ============================================================
void StatsPanel::buildHistogramChart()
{
    QChart* chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(Theme::BG_PANEL)));
    chart->setBackgroundRoundness(6);
    chart->setMargins(QMargins(4,4,4,4));
    chart->legend()->hide();
    chart->setTitle("");

    m_histSeries = new QBarSeries();
    QBarSet* bs = new QBarSet("Count");
    bs->setColor(QColor(0, 229, 255, 153));
    bs->setBorderColor(QColor(0, 100, 130));
    *bs << 0;
    m_histSeries->append(bs);
    chart->addSeries(m_histSeries);

    m_histAxisX = new QBarCategoryAxis();
    m_histAxisX->setLabelsColor(QColor(Theme::TEXT_DIM));
    m_histAxisX->setGridLineColor(QColor(Theme::BORDER));
    m_histAxisX->setLinePenColor(QColor(Theme::BORDER));
    chart->addAxis(m_histAxisX, Qt::AlignBottom);
    m_histSeries->attachAxis(m_histAxisX);

    m_histAxisY = new QValueAxis();
    m_histAxisY->setLabelsColor(QColor(Theme::TEXT_DIM));
    m_histAxisY->setGridLineColor(QColor(Theme::BORDER));
    m_histAxisY->setLinePenColor(QColor(Theme::BORDER));
    chart->addAxis(m_histAxisY, Qt::AlignLeft);
    m_histSeries->attachAxis(m_histAxisY);

    m_histView = new QChartView(chart);
    m_histView->setRenderHint(QPainter::Antialiasing);
    m_histView->setMinimumHeight(180);
    m_histView->setMaximumHeight(200);
    m_histView->setBackgroundBrush(QBrush(QColor(Theme::BG_PANEL)));
}

void StatsPanel::buildRmsChart()
{
    QChart* chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(Theme::BG_PANEL)));
    chart->setBackgroundRoundness(6);
    chart->setMargins(QMargins(4,4,4,4));
    chart->setTitle("");

    m_rmsLine  = new QLineSeries();
    m_sigxLine = new QLineSeries();
    m_sigzLine = new QLineSeries();
    m_rmsMark  = new QScatterSeries();

    QPen rPen(QColor(Theme::GOLD), 2);
    QPen xPen(QColor(Theme::CYAN), 1.5);
    QPen zPen(QColor(Theme::BLUE_FIELD), 1.5);
    xPen.setStyle(Qt::DashLine);
    zPen.setStyle(Qt::DotLine);

    m_rmsLine->setPen(rPen);
    m_sigxLine->setPen(xPen);
    m_sigzLine->setPen(zPen);
    m_rmsLine->setName("r_rms");
    m_sigxLine->setName("σ_x");
    m_sigzLine->setName("σ_z");

    m_rmsMark->setColor(QColor(Theme::GOLD));
    m_rmsMark->setMarkerSize(8);
    m_rmsMark->setName("Current");

    chart->addSeries(m_rmsLine);
    chart->addSeries(m_sigxLine);
    chart->addSeries(m_sigzLine);
    chart->addSeries(m_rmsMark);

    m_rmsAxisX = new QValueAxis();
    m_rmsAxisX->setTitleText("Time");
    m_rmsAxisX->setTitleBrush(QBrush(QColor(Theme::TEXT_DIM)));
    m_rmsAxisX->setLabelsColor(QColor(Theme::TEXT_DIM));
    m_rmsAxisX->setGridLineColor(QColor(Theme::BORDER));

    m_rmsAxisY = new QValueAxis();
    m_rmsAxisY->setTitleText("cm");
    m_rmsAxisY->setTitleBrush(QBrush(QColor(Theme::TEXT_DIM)));
    m_rmsAxisY->setLabelsColor(QColor(Theme::TEXT_DIM));
    m_rmsAxisY->setGridLineColor(QColor(Theme::BORDER));

    chart->addAxis(m_rmsAxisX, Qt::AlignBottom);
    chart->addAxis(m_rmsAxisY, Qt::AlignLeft);
    m_rmsLine->attachAxis(m_rmsAxisX);
    m_rmsLine->attachAxis(m_rmsAxisY);
    m_sigxLine->attachAxis(m_rmsAxisX);
    m_sigxLine->attachAxis(m_rmsAxisY);
    m_sigzLine->attachAxis(m_rmsAxisX);
    m_sigzLine->attachAxis(m_rmsAxisY);
    m_rmsMark->attachAxis(m_rmsAxisX);
    m_rmsMark->attachAxis(m_rmsAxisY);

    QLegend* leg = chart->legend();
    leg->setAlignment(Qt::AlignBottom);
    leg->setLabelColor(QColor(Theme::TEXT_DIM));
    leg->setBrush(QBrush(QColor(Theme::BG_PANEL)));
    leg->setPen(QPen(QColor(Theme::BORDER)));

    m_rmsView = new QChartView(chart);
    m_rmsView->setRenderHint(QPainter::Antialiasing);
    m_rmsView->setMinimumHeight(200);
    m_rmsView->setMaximumHeight(220);
    m_rmsView->setBackgroundBrush(QBrush(QColor(Theme::BG_PANEL)));
}

// ============================================================
void StatsPanel::setData(const SimulationData* data)
{
    m_data = data;
    if (!data) return;

    // ── Summary cards
    m_lblTotal->setText(QString::number(data->n_particles));
    m_lblDetected->setText(QString::number(data->n_detected));
    m_lblLost->setText(QString::number(data->n_lost));
    m_lblEffic->setText(QString("%1%").arg(data->efficiency_pct, 0, 'f', 1));
    m_lblTubeR->setText(QString("%1 m").arg(data->tube_radius, 0, 'f', 3));
    m_lblTimeRange->setText(QString("%1–%2 %3")
        .arg(data->t_min_ns, 0, 'f', 1)
        .arg(data->t_max_ns, 0, 'f', 1)
        .arg(QString::fromStdString(data->time_unit())));

    // ── RMS chart: populate all frames
    m_rmsLine->clear();
    m_sigxLine->clear();
    m_sigzLine->clear();
    float maxRms = 0.0f;
    for (const auto& s : data->stats) {
        double t = data->time_to_ns(s.time);
        m_rmsLine->append(t, s.r_rms_cm);
        m_sigxLine->append(t, s.sigma_x_cm);
        m_sigzLine->append(t, s.sigma_z_cm);
        maxRms = std::max(maxRms, std::max(s.r_rms_cm, std::max(s.sigma_x_cm, s.sigma_z_cm)));
    }
    m_rmsAxisX->setRange(data->t_min_ns, data->t_max_ns);
    m_rmsAxisX->setTitleText(QString("Time [%1]").arg(QString::fromStdString(data->time_unit())));
    m_rmsAxisY->setRange(0, maxRms * 1.15f);

    // ── Frame stats table
    m_frameTable->setColumnCount(7);
    m_frameTable->setHorizontalHeaderLabels({
        "Time", "In-flight", "Detected", "Lost", "r_rms [cm]", "σ_x [cm]", "η [%]"
    });
    m_frameTable->setRowCount(data->n_timesteps);
    for (int f = 0; f < data->n_timesteps; ++f) {
        const FrameStats& s = data->stats[f];
        auto setCell = [&](int col, const QString& txt) {
            QTableWidgetItem* it = new QTableWidgetItem(txt);
            it->setTextAlignment(Qt::AlignCenter);
            m_frameTable->setItem(f, col, it);
        };
        setCell(0, QString("%1").arg(data->time_to_ns(s.time), 0, 'f', 3));
        setCell(1, QString::number(s.in_flight));
        setCell(2, QString::number(s.detected));
        setCell(3, QString::number(s.lost));
        setCell(4, QString("%1").arg(s.r_rms_cm, 0, 'f', 3));
        setCell(5, QString("%1").arg(s.sigma_x_cm, 0, 'f', 3));
        setCell(6, QString("%1").arg(s.efficiency, 0, 'f', 2));
    }
    m_frameTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_frameTable->verticalHeader()->setDefaultSectionSize(22);

    // ── Metadata table
    struct KV { QString key, val; };
    std::vector<KV> entries = {
        {"Model Name",   QString::fromStdString(data->meta.model_name)},
        {"COMSOL Ver.",  QString::fromStdString(data->meta.version)},
        {"Export Date",  QString::fromStdString(data->meta.date)},
        {"Dimension",    QString::number(data->meta.dimension)},
        {"Mesh Nodes",   QString::number(data->meta.nodes)},
        {"Expressions",  QString::number(data->meta.expressions)},
        {"Description",  QString::fromStdString(data->meta.description)},
        {"Particles",    QString::number(data->n_particles)},
        {"Timesteps",    QString::number(data->n_timesteps)},
        {"Tube Radius",  QString("%1 m").arg(data->tube_radius, 0, 'f', 4)},
        {"Y range",      QString("[%1, %2] m").arg(data->y_min,0,'f',3).arg(data->y_max,0,'f',3)},
        {"Detector Y",   QString("%1 m").arg(data->meta_y, 0, 'f', 4)},
        {"Final Eff.",   QString("%1 %").arg(data->efficiency_pct, 0, 'f', 2)},
    };
    m_metaTable->setColumnCount(2);
    m_metaTable->setHorizontalHeaderLabels({"Parameter", "Value"});
    m_metaTable->setRowCount((int)entries.size());
    for (int i = 0; i < (int)entries.size(); ++i) {
        auto k = new QTableWidgetItem(entries[i].key);
        k->setForeground(QBrush(QColor(Theme::TEXT_DIM)));
        auto v = new QTableWidgetItem(entries[i].val);
        v->setForeground(QBrush(QColor(Theme::TEXT_PRIMARY)));
        m_metaTable->setItem(i, 0, k);
        m_metaTable->setItem(i, 1, v);
    }
    m_metaTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_metaTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_metaTable->verticalHeader()->setDefaultSectionSize(22);

    updateFrame(0);
}

// ============================================================
void StatsPanel::updateFrame(int frame)
{
    if (!m_data || frame >= m_data->n_timesteps) return;
    const FrameStats& s = m_data->stats[frame];

    // Live counters
    m_lblInFlight->setText(QString("%1").arg(s.in_flight));
    m_lblDetNow->setText(QString("%1").arg(s.detected));
    m_lblLostNow->setText(QString("%1").arg(s.lost));
    m_lblEffNow->setText(QString("%1%").arg(s.efficiency, 0, 'f', 1));
    m_lblTimeNow->setText(QString("%1 %2")
        .arg(m_data->time_to_ns(s.time), 0, 'f', 2)
        .arg(QString::fromStdString(m_data->time_unit())));
    m_lblRmsNow->setText(QString("%1 cm").arg(s.r_rms_cm, 0, 'f', 2));

    // Scroll frame table to current row
    m_frameTable->selectRow(frame);
    m_frameTable->scrollTo(m_frameTable->model()->index(frame, 0));

    updateHistogram(frame);
    updateRmsChart(frame);
}

void StatsPanel::updateHistogram(int frame)
{
    if (!m_data) return;
    const int NBINS = 30;
    float R = m_data->tube_radius;
    std::vector<int> counts(NBINS, 0);

    for (const auto& p : m_data->particles) {
        if (frame >= p.stop_frame) continue;  // stopped
        if (frame >= (int)p.frames.size()) continue;
        float r = p.frames[frame].pos.r_transverse();
        int bin = (int)(r / R * NBINS);
        bin = std::min(bin, NBINS - 1);
        ++counts[bin];
    }

    // Rebuild bar set
    QBarSeries* newSeries = new QBarSeries();
    QBarSet* bs = new QBarSet("Count");
    bs->setColor(QColor(0, 229, 255, 153));
    bs->setBorderColor(QColor(0, 100, 130));

    QStringList cats;
    int maxCount = 1;
    for (int i = 0; i < NBINS; ++i) {
        *bs << counts[i];
        cats << QString("%1").arg(R / NBINS * (i + 0.5f) * 100, 0, 'f', 0);
        maxCount = std::max(maxCount, counts[i]);
    }
    newSeries->append(bs);

    QChart* chart = m_histView->chart();
    chart->removeAllSeries();
    chart->addSeries(newSeries);
    m_histSeries = newSeries;
    newSeries->attachAxis(m_histAxisX);
    newSeries->attachAxis(m_histAxisY);
    m_histAxisX->setCategories(cats);
    m_histAxisY->setRange(0, maxCount * 1.1);
    m_histAxisX->setTitleText(QString("r [cm]  |  R_tube=%1 cm").arg(R*100, 0,'f',1));
}

void StatsPanel::updateRmsChart(int frame)
{
    if (!m_data || m_rmsLine->points().empty()) return;
    m_rmsMark->clear();
    double t = m_data->time_to_ns(m_data->stats[frame].time);
    double rms = m_data->stats[frame].r_rms_cm;
    m_rmsMark->append(t, rms);
}
