#pragma once

#include <stdexcept>
#include <vector>
#include <cmath>

#include "boris/simulation/constants.hpp"
#include "boris/simulation/field_operators.hpp"
#include "boris/mechanisms/cic.hpp"

namespace boris {

struct DepositedMoments {
    std::vector<double> density;
    std::vector<Vec3> bulk_velocity;
};

// CIC-deposit particle weights and weighted velocities, then normalize the
// velocity moment by density to obtain the grid bulk velocity.
// N^n=sum 
// here particle weight is one, while particle.charge is defined as one, thus particle weight used charge
inline DepositedMoments deposit_particle_moments(const PeriodicGrid& grid,
                                                 const std::vector<Particle>& particles) {
    DepositedMoments result{std::vector<double>(grid.size(), 0.0),
                            std::vector<Vec3>(grid.size(), Vec3{})};
    for (const Particle& particle : particles) {
        if (!particle.alive) continue;
        const double particle_weight = std::abs(particle.charge);
        for (const CicPointWeight& item : cic_weights(grid, particle.position)) {
            const double weighted = particle_weight * item.weight;
            result.density[item.cell] += weighted;
            result.bulk_velocity[item.cell] =
                add(result.bulk_velocity[item.cell], multiply(particle.velocity, weighted));
        }
    }
    // normalized u - mean velocity
    for (std::size_t cell = 0; cell < grid.size(); ++cell) {
        if (result.density[cell] > 0.0) {
            result.bulk_velocity[cell] = multiply(result.bulk_velocity[cell],
                                                  1.0 / result.density[cell]);
        }
    }
    return result;
}

// Faraday's law: B(t + dt) = B(t) - dt * curl(E).
inline std::vector<Vec3> faraday(const PeriodicGrid& grid,
                                 const std::vector<Vec3>& magnetic,
                                 const std::vector<Vec3>& electric,
                                 double dt) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (magnetic.size() != grid.size() || electric.size() != grid.size()) {
        throw std::invalid_argument("field arrays must match grid size");
    }
    const std::vector<Vec3> curl_e = curl_periodic(grid, electric);
    std::vector<Vec3> result(grid.size());
    for (std::size_t i = 0; i < grid.size(); ++i) {
        result[i] = subtract(magnetic[i], multiply(curl_e[i], dt));
    }
    return result;
}

// Generalized Ohm's law for isotropic, isothermal electrons:
// E = -u x B + (curl B)/N - Te grad(ln N) + eta curl B - nu laplacian(curl B).
// Te is constant and Pe = N Te, so div(Pe)/N = Te grad(ln N).
inline std::vector<Vec3> compute_electric(
    const PeriodicGrid& grid,
    const std::vector<Vec3>& magnetic,
    const std::vector<double>& density,
    const std::vector<Vec3>& bulk_velocity,
    const SimulationConstants& constants = default_constants) {
    if (magnetic.size() != grid.size() || density.size() != grid.size() ||
        bulk_velocity.size() != grid.size()) {
        throw std::invalid_argument("field arrays must match grid size");
    }

    const std::vector<Vec3> curl_b = curl_periodic(grid, magnetic);
    const std::vector<Vec3> grad_n = gradient_periodic(grid, density);
    const std::vector<Vec3> lap_curl_b = laplacian_periodic(grid, curl_b);
    std::vector<Vec3> result(grid.size());
    for (std::size_t i = 0; i < grid.size(); ++i) {
        const double n = density[i];
        if (!(n > 0.0)) throw std::invalid_argument("density must be positive");
        Vec3 value = multiply(cross(bulk_velocity[i], magnetic[i]), -1.0);
        value = add(value, multiply(curl_b[i], 1.0 / n));
        value = subtract(value, multiply(grad_n[i], constants.electron_temperature / n));
        value = add(value, multiply(curl_b[i], constants.resistivity));
        value = subtract(value, multiply(lap_curl_b[i], constants.viscosity));
        result[i] = value;
    }
    return result;
}

}  // namespace boris
