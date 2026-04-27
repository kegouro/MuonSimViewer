#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <unordered_map>

// ============================================================
//  SIMULATION DATA STRUCTURES
//  Generic COMSOL 3D particle tracking data
// ============================================================

struct Vec3 {
    float x = 0, y = 0, z = 0;
    float norm() const { return std::sqrt(x*x + y*y + z*z); }
    float r_transverse() const { return std::sqrt(x*x + z*z); }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
};

// Per-particle, per-timestep state
struct ParticleFrame {
    Vec3   pos;
    float  time = 0.0f;
};

// One complete particle trajectory
struct Particle {
    int    id           = 0;
    std::vector<ParticleFrame> frames;

    // Derived (computed after loading)
    bool   reached_detector = false;
    bool   hit_wall         = false;
    int    stop_frame       = -1;   // frame where it froze (-1 = never)
    float  r_initial        = 0.0f; // transverse radius at t=0
    float  r_norm           = 0.0f; // normalized r_init ∈ [0,1]
};

// Aggregated statistics per timestep
struct FrameStats {
    float time       = 0.0f;
    int   in_flight  = 0;
    int   detected   = 0;
    int   lost       = 0;
    float r_rms_cm   = 0.0f;
    float sigma_x_cm = 0.0f;
    float sigma_z_cm = 0.0f;
    float efficiency = 0.0f;
};

// Metadata parsed from COMSOL CSV header comments
struct ComsolMetadata {
    std::string model_name;
    std::string version;
    std::string date;
    int         dimension    = 0;
    int         nodes        = 0;
    int         expressions  = 0;
    std::string description;
    std::vector<std::string> column_names;
};

// Complete simulation dataset
struct SimulationData {
    ComsolMetadata meta;
    std::vector<Particle> particles;
    std::vector<float>    timesteps;    // unique time values [s]
    std::vector<FrameStats> stats;      // one per timestep

    // Geometry
    float tube_radius    = 0.25f;  // [m]
    float y_min          = 0.0f;
    float y_max          = 0.0f;
    float meta_y         = 0.0f;   // detector plane Y

    // Summary
    int   n_particles    = 0;
    int   n_timesteps    = 0;
    int   n_detected     = 0;
    int   n_lost         = 0;
    float efficiency_pct = 0.0f;
    float t_min_ns       = 0.0f;
    float t_max_ns       = 0.0f;

    bool is_valid() const { return !particles.empty() && !timesteps.empty(); }

    // Time in nanoseconds
    float time_to_ns(float t_s) const {
        // COMSOL typically outputs in seconds; scale to ns if values < 1e-6
        if (!timesteps.empty() && timesteps.back() < 1e-6f)
            return t_s * 1e9f;
        return t_s;
    }

    std::string time_unit() const {
        if (!timesteps.empty() && timesteps.back() < 1e-6f) return "ns";
        if (!timesteps.empty() && timesteps.back() < 1e-3f) return "µs";
        return "s";
    }
};
