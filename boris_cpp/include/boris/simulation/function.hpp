#pragma once

#include <stdexcept>
#include <vector>

#include "boris/simulation/constants.hpp"
#include "boris/simulation/field_operators.hpp"

namespace boris {

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
// E = -u x B + (curl B)/N - Te grad(ln N) + eta curl B - nu laplacian(B).
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
    const std::vector<Vec3> lap_b = laplacian_periodic(grid, magnetic);
    std::vector<Vec3> result(grid.size());
    for (std::size_t i = 0; i < grid.size(); ++i) {
        const double n = density[i];
        if (!(n > 0.0)) throw std::invalid_argument("density must be positive");
        Vec3 value = multiply(cross(bulk_velocity[i], magnetic[i]), -1.0);
        value = add(value, multiply(curl_b[i], 1.0 / n));
        value = subtract(value, multiply(grad_n[i], constants.electron_temperature / n));
        value = add(value, multiply(curl_b[i], constants.resistivity));
        value = subtract(value, multiply(lap_b[i], constants.viscosity));
        result[i] = value;
    }
    return result;
}

}  // namespace boris
