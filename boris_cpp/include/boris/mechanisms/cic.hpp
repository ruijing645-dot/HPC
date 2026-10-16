#pragma once

#include <vector>

#include "boris/core/grid_utils.hpp"

namespace boris {

struct CicPointWeight {
    std::size_t cell;
    double weight;
};

// Return the four node-centered CIC weights in the x-y plane.
inline std::vector<CicPointWeight> cic_weights(const PeriodicGrid& grid, Vec3 position) {
    const Vec3 point = grid.wrap_position(position);
    const Vec3 dx = grid.spacing();
    const double gx = (point.x - grid.origin.x) / dx.x;
    const double gy = (point.y - grid.origin.y) / dx.y;
    const int i0 = static_cast<int>(std::floor(gx));
    const int j0 = static_cast<int>(std::floor(gy));
    const double fx = gx - i0;
    const double fy = gy - j0;

    std::vector<CicPointWeight> result;
    result.reserve(4);
    for (int di = 0; di <= 1; ++di) {
        for (int dj = 0; dj <= 1; ++dj) {
            const double wx = di ? fx : 1.0 - fx;
            const double wy = dj ? fy : 1.0 - fy;
            result.push_back({grid.index(i0 + di, j0 + dj, 0), wx * wy});
        }
    }
    return result;
}

inline Vec3 interpolate_cic(const PeriodicGrid& grid, const std::vector<Vec3>& values,
                            Vec3 position) {
    if (values.size() != grid.size()) {
        throw std::invalid_argument("grid value count does not match grid size");
    }
    Vec3 result{};
    const auto weights = cic_weights(grid, position);

    for (const CicPointWeight& item : weights) {
        result = add(result, multiply(values[item.cell], item.weight));
    }
    return result;
}

}  // namespace boris
