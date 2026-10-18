#pragma once

#include <stdexcept>
#include "boris/simulation/constants.hpp"
#include "boris/simulation/function.hpp"
#include "boris/simulation/simulation_types.hpp"

namespace boris {

// Predictor: build old-time moments, predict B and E, then advance particles
// using fields averaged between time levels and sampled at the half-step position.
inline PredictionResult predict_first(const PeriodicGrid& grid, const PredictionInput& input,
                                      double dt,
                                      const SimulationConstants& constants = default_constants) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (input.electric.size() != grid.size() || input.magnetic.size() != grid.size()) {
        throw std::invalid_argument("electric and magnetic arrays must match grid size");
    }

    PredictionResult result;
    DepositedMoments old_moments = deposit_particle_moments(grid, input.particles);
    double density_sum = 0.0;
    for (double n : old_moments.density) density_sum += n;
    if (!(density_sum > 0.0)) throw std::invalid_argument("prediction requires at least one live particle");
    const double density_floor = density_sum / static_cast<double>(grid.size()) * 1e-12;
    for (double& density : old_moments.density)
        if (!(density > 0.0)) density = density_floor;

    result.magnetic_next = faraday(grid, input.magnetic, input.electric, dt);
    const std::vector<Vec3> electric_next =
        compute_electric(grid, result.magnetic_next, old_moments.density,
                         old_moments.bulk_velocity, constants);
    result.electric_half.resize(grid.size());
    std::vector<Vec3> magnetic_half(grid.size());
    for (std::size_t cell = 0; cell < grid.size(); ++cell) {
        result.electric_half[cell] = multiply(add(input.electric[cell], electric_next[cell]), 0.5);
        magnetic_half[cell] = multiply(add(input.magnetic[cell], result.magnetic_next[cell]), 0.5);
    }

    result.particles_next = input.particles;
    for (Particle& particle : result.particles_next) {
        if (!particle.alive) continue;
        const Vec3 old_position = particle.position;
        Vec3 half_position = add(old_position, multiply(particle.velocity, 0.5 * dt));
        half_position.z = old_position.z;
        half_position = grid.wrap_position(half_position);
        const Vec3 electric = interpolate_cic(grid, result.electric_half, half_position);
        const Vec3 magnetic = interpolate_cic(grid, magnetic_half, half_position);
        push(particle, dt, electric, magnetic);
        Vec3 next_position = add(old_position, multiply(particle.velocity, dt));
        next_position.z = old_position.z;
        particle.position = grid.wrap_position(next_position);
    }
    DepositedMoments next_moments = deposit_particle_moments(grid, result.particles_next);
    result.density_next = next_moments.density;
    result.bulk_velocity_next = next_moments.bulk_velocity;
    return result;
}

}  // namespace boris
