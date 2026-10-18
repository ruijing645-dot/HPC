#pragma once

#include <stdexcept>

#include "boris/simulation/function.hpp"
#include "boris/simulation/simulation_types.hpp"

namespace boris {

// Correct magnetic and electric fields after the prediction passes.
// electron_pressure is the grid representation of grad(P_e).
inline CorrectionResult correct_fields(const PeriodicGrid& grid, const CorrectionInput& input,
                                       double dt, double eta, double nu) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    const std::size_t count = grid.size();
    if (input.magnetic.size() != count || input.electric_half.size() != count ||
        input.density.size() != count || input.bulk_velocity.size() != count ||
        input.electron_pressure.size() != count) {
        throw std::invalid_argument("correction arrays must match grid size");
    }

    CorrectionResult result;
    result.magnetic = faraday(grid, input.magnetic, input.electric_half, dt);
    const std::vector<Vec3> curl_b = curl_periodic(grid, result.magnetic);
    const std::vector<Vec3> lap_b = laplacian_periodic(grid, result.magnetic);
    result.electric.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        if (!(input.density[i] > 0.0)) {
            throw std::invalid_argument("density must be positive for correction");
        }
        Vec3 value = multiply(cross(input.bulk_velocity[i], result.magnetic[i]), -1.0);
        value = add(value, multiply(curl_b[i], 1.0 / input.density[i]));
        value = subtract(value, multiply(input.electron_pressure[i], 1.0 / input.density[i]));
        value = add(value, multiply(curl_b[i], eta));
        value = subtract(value, multiply(lap_b[i], nu));
        result.electric[i] = value;
    }
    return result;
}

}  // namespace boris
