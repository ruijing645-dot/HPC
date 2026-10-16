#pragma once

#include <stdexcept>
#include <vector>

#include "boris/simulation/field_operators.hpp"

namespace boris {

// Faraday's law: B(t + dt) = B(t) - dt * curl(E).
inline std::vector<Vec3> advance_magnetic_faraday(const PeriodicGrid& grid,
                                                  const std::vector<Vec3>& magnetic,
                                                  const std::vector<Vec3>& electric,
                                                  double dt) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (magnetic.size() != grid.size()) {
        throw std::invalid_argument("magnetic array must match grid size");
    }

    const std::vector<Vec3> curl_e = curl_periodic(grid, electric);
    std::vector<Vec3> result(grid.size());
    for (std::size_t i = 0; i < grid.size(); ++i) {
        result[i] = subtract(magnetic[i], multiply(curl_e[i], dt));
    }
    return result;
}

}  // namespace boris
