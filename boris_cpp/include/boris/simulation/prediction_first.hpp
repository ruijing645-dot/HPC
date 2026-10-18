#pragma once

#include <stdexcept>
#include <cmath>

#include "boris/mechanisms/cic.hpp"
#include "boris/simulation/constants.hpp"
#include "boris/simulation/function.hpp"
#include "boris/simulation/simulation_types.hpp"

namespace boris {

// First prediction: update B with Faraday's law, push particles, and deposit
// charge density and bulk velocity on the 2D grid.
inline PredictionResult predict_first(const PeriodicGrid& grid, const PredictionInput& input,
                                      double dt,
                                      const SimulationConstants& constants = default_constants) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (input.electric.size() != grid.size() || input.magnetic.size() != grid.size()) {
        throw std::invalid_argument("electric and magnetic arrays must match grid size");
    }

    PredictionResult result;
    result.magnetic_next = faraday(grid, input.magnetic, input.electric, dt);
    result.particles_next = input.particles;
    result.density_next.assign(grid.size(), 0.0);
    result.bulk_velocity_next.assign(grid.size(), Vec3{});

    for (Particle& particle : result.particles_next) {
        if (!particle.alive) continue;
        const Vec3 electric = interpolate_cic(grid, input.electric, particle.position);
        const Vec3 magnetic = interpolate_cic(grid, input.magnetic, particle.position);
        push(particle, dt, electric, magnetic);

        Vec3 next_position = add(particle.position, multiply(particle.velocity, dt));
        next_position.z = particle.position.z;
        particle.position = grid.wrap_position(next_position);

        for (const CicPointWeight& item : cic_weights(grid, particle.position)) {
            const double weighted_charge = particle.charge * item.weight;
            result.density_next[item.cell] += weighted_charge;
            result.bulk_velocity_next[item.cell] =
                add(result.bulk_velocity_next[item.cell], multiply(particle.velocity, weighted_charge));
        }
    }

    for (std::size_t cell = 0; cell < grid.size(); ++cell) {
        if (result.density_next[cell] != 0.0) {
            result.bulk_velocity_next[cell] = multiply(result.bulk_velocity_next[cell],
                                                       1.0 / result.density_next[cell]);
        }
    }
    std::vector<double> number_density = result.density_next;
    for (double& value : number_density) value = std::abs(value);
    result.electric_half = compute_electric(grid, input.magnetic, number_density,
                                            result.bulk_velocity_next, constants);
    return result;
}

}  // namespace boris
