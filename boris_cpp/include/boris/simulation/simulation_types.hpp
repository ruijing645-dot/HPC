#pragma once

#include <vector>

#include "boris/mechanisms/boris.hpp"

namespace boris {

struct PredictionInput {
    std::vector<Vec3> electric;
    std::vector<Vec3> magnetic;
    std::vector<Particle> particles;
};

struct PredictionResult {
    std::vector<Vec3> magnetic_next;
    std::vector<Vec3> electric_half;
    std::vector<Particle> particles_next;
    std::vector<double> density_next;
    std::vector<Vec3> bulk_velocity_next;
};

struct CorrectionInput {
    std::vector<Vec3> magnetic;
    std::vector<Vec3> electric_half;
    std::vector<double> density;
    std::vector<Vec3> bulk_velocity;
    std::vector<Vec3> electron_pressure;
};

struct CorrectionResult {
    std::vector<Vec3> magnetic;
    std::vector<Vec3> electric;
};

}  // namespace boris
