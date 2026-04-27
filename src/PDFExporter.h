#pragma once
#include "SimulationData.h"
#include "Viewport3D.h"
#include <QString>
#include <functional>

// ============================================================
//  PDF EXPORTER — full scientific report
// ============================================================
class PDFExporter {
public:
    using ProgressCb = std::function<void(int, const QString&)>;
    static bool exportReport(const QString& filepath,
                             const SimulationData* data,
                             Viewport3D* beam,
                             Viewport3D* traj,
                             int currentFrame,
                             ProgressCb cb = nullptr);
};
