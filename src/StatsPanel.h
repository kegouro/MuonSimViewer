#pragma once
#include "SimulationData.h"
#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QtCharts>

// ============================================================
//  STATS PANEL — Scrollable statistics dashboard
//  Shows real-time counter, histogram, RMS charts,
//  simulation metadata table and per-frame statistics table
// ============================================================

class StatsPanel : public QScrollArea
{
    Q_OBJECT
public:
    explicit StatsPanel(QWidget* parent = nullptr);
    void setData(const SimulationData* data);
    void updateFrame(int frame);

private:
    void buildUI();
    void buildMetaTable();
    void buildSummaryCards();
    void buildParticleCounters();
    void buildCharts();
    void buildFrameStatsTable();
    void buildHistogramChart();
    void buildRmsChart();

    void updateCounters(int frame);
    void updateHistogram(int frame);
    void updateRmsChart(int frame);
    void updateFrameTable(int frame);

    const SimulationData* m_data = nullptr;

    // Summary cards
    QLabel* m_lblTotal     = nullptr;
    QLabel* m_lblDetected  = nullptr;
    QLabel* m_lblLost      = nullptr;
    QLabel* m_lblEffic     = nullptr;
    QLabel* m_lblTubeR     = nullptr;
    QLabel* m_lblTimeRange = nullptr;

    // Dynamic counters
    QLabel* m_lblInFlight  = nullptr;
    QLabel* m_lblDetNow    = nullptr;
    QLabel* m_lblLostNow   = nullptr;
    QLabel* m_lblEffNow    = nullptr;
    QLabel* m_lblTimeNow   = nullptr;
    QLabel* m_lblRmsNow    = nullptr;

    // Charts
    QChartView* m_histView  = nullptr;
    QChartView* m_rmsView   = nullptr;

    QBarSeries*       m_histSeries = nullptr;
    QBarCategoryAxis* m_histAxisX  = nullptr;
    QValueAxis*       m_histAxisY  = nullptr;

    QLineSeries*   m_rmsLine    = nullptr;
    QLineSeries*   m_sigxLine   = nullptr;
    QLineSeries*   m_sigzLine   = nullptr;
    QScatterSeries* m_rmsMark   = nullptr;
    QValueAxis*    m_rmsAxisX   = nullptr;
    QValueAxis*    m_rmsAxisY   = nullptr;

    // Tables
    QTableWidget* m_metaTable  = nullptr;
    QTableWidget* m_frameTable = nullptr;

    // Main content widget
    QWidget* m_content = nullptr;
};
