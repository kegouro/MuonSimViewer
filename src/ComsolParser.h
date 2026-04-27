#pragma once
#include "SimulationData.h"
#include <QString>
#include <functional>

// ============================================================
//  COMSOL CSV PARSER
//  Handles any 3D particle tracking export from COMSOL
//  with columns: qx, qy, qz, cpt.pidx, t  (per timestep)
// ============================================================

class ComsolParser {
public:
    // Progress callback: (percent 0-100, message)
    using ProgressCb = std::function<void(int, const QString&)>;

    static SimulationData parse(const QString& filepath,
                                ProgressCb progress = nullptr);

    // Returns last error message
    static QString lastError();

private:
    static QString s_last_error;

    static void computeDerivedData(SimulationData& data);
    static void computeFrameStats(SimulationData& data);
    static void classifyParticles(SimulationData& data);
    static void autoDetectTubeRadius(SimulationData& data);
};
