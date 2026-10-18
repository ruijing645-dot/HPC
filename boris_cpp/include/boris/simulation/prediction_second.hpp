#pragma once

#include <stdexcept>
#include <utility>

#include "boris/simulation/constants.hpp"
#include "boris/simulation/function.hpp"
#include "boris/simulation/simulation_types.hpp"

namespace boris {

// Second predictor: continue from the first predictor's fields and particles,
// then repeat the same field prediction, particle push, and CIC deposition.
inline PredictionResult predict_second(const PeriodicGrid& grid, const PredictionInput& input,
                                       const PredictionResult& first, double dt,
                                       const SimulationConstants& constants = default_constants) {
    (void)input;  // Kept for API symmetry with predict_first; the second stage starts from `first`.
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (first.electric_half.size() != grid.size() || first.magnetic_next.size() != grid.size()) {
        throw std::invalid_argument("first-prediction field arrays must match grid size");
    }

    PredictionResult result;
    DepositedMoments old_moments = deposit_particle_moments(grid, first.particles_next);
    double density_sum = 0.0;
    for (double n : old_moments.density) density_sum += n;
    if (!(density_sum > 0.0)) {
        throw std::invalid_argument("second prediction requires at least one live particle");
    }
    const double density_floor = density_sum / static_cast<double>(grid.size()) * 1e-12;
    for (double& density : old_moments.density) {
        if (!(density > 0.0)) density = density_floor;
    }

    result.magnetic_next = faraday(grid, first.magnetic_next, first.electric_half, dt);
    const std::vector<Vec3> electric_next =
        compute_electric(grid, result.magnetic_next, old_moments.density,
                         old_moments.bulk_velocity, constants);
    result.electric_half.resize(grid.size());
    std::vector<Vec3> magnetic_half(grid.size());
    for (std::size_t cell = 0; cell < grid.size(); ++cell) {
        result.electric_half[cell] =
            multiply(add(first.electric_half[cell], electric_next[cell]), 0.5);
        magnetic_half[cell] =
            multiply(add(first.magnetic_next[cell], result.magnetic_next[cell]), 0.5);
    }

    result.particles_next = first.particles_next;
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
    result.density_next = std::move(next_moments.density);
    result.bulk_velocity_next = std::move(next_moments.bulk_velocity);
    return result;
}

}  // namespace boris
