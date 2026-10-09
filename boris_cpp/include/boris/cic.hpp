#pragma once

#include <array>
#include <vector>

#include "boris/grid_utils.hpp"

namespace boris {

struct CicPointWeight {
    std::size_t cell;
    double weight;
};

// Return the eight cell-centered CIC weights. PeriodicGrid wraps edge cells.
inline std::array<CicPointWeight, 8> cic_weights(const PeriodicGrid& grid, Vec3 position) {
    const Vec3 point = grid.wrap_position(position);
    const Vec3 dx = grid.spacing();
    const double gx = (point.x - grid.origin.x) / dx.x - 0.5;
    const double gy = (point.y - grid.origin.y) / dx.y - 0.5;
    const double gz = (point.z - grid.origin.z) / dx.z - 0.5;
    const int i0 = static_cast<int>(std::floor(gx));
    const int j0 = static_cast<int>(std::floor(gy));
    const int k0 = static_cast<int>(std::floor(gz));
    const double fx = gx - i0;
    const double fy = gy - j0;
    const double fz = gz - k0;

    std::array<CicPointWeight, 8> result{};
    int n = 0;
    for (int di = 0; di <= 1; ++di) {
        for (int dj = 0; dj <= 1; ++dj) {
            for (int dk = 0; dk <= 1; ++dk) {
                const double wx = di ? fx : 1.0 - fx;
                const double wy = dj ? fy : 1.0 - fy;
                const double wz = dk ? fz : 1.0 - fz;
                result[n++] = {grid.index(i0 + di, j0 + dj, k0 + dk), wx * wy * wz};
            }
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
    for (const CicPointWeight& item : cic_weights(grid, position)) {
        result = add(result, multiply(values[item.cell], item.weight));
    }
    return result;
}

}  // namespace boris
