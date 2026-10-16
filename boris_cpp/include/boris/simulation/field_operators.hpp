#pragma once

#include <stdexcept>
#include <vector>

#include "boris/core/grid_utils.hpp"

namespace boris {

// Curl for a 2D3V field: all three vector components are kept, while d/dz=0.
inline std::vector<Vec3> curl_periodic(const PeriodicGrid& grid,
                                       const std::vector<Vec3>& field) {
    if (field.size() != grid.size()) throw std::invalid_argument("field array must match grid size");
    const Vec3 dx = grid.spacing();
    std::vector<Vec3> result(grid.size());
    for (int i = 0; i < grid.cells[0]; ++i) {
        for (int j = 0; j < grid.cells[1]; ++j) {
            const std::size_t c = grid.index(i, j, 0);
            const Vec3 xp = field[grid.index(i + 1, j, 0)];
            const Vec3 xm = field[grid.index(i - 1, j, 0)];
            const Vec3 yp = field[grid.index(i, j + 1, 0)];
            const Vec3 ym = field[grid.index(i, j - 1, 0)];
            result[c] = {(yp.z - ym.z) / (2.0 * dx.y),
                         -(xp.z - xm.z) / (2.0 * dx.x),
                         (xp.y - xm.y) / (2.0 * dx.x) - (yp.x - ym.x) / (2.0 * dx.y)};
        }
    }
    return result;
}

// Component-wise Laplacian in the x-y plane; z derivatives are zero.
inline std::vector<Vec3> laplacian_periodic(const PeriodicGrid& grid,
                                            const std::vector<Vec3>& field) {
    if (field.size() != grid.size()) throw std::invalid_argument("field array must match grid size");
    const Vec3 dx = grid.spacing();
    std::vector<Vec3> result(grid.size());
    for (int i = 0; i < grid.cells[0]; ++i) {
        for (int j = 0; j < grid.cells[1]; ++j) {
            const std::size_t c = grid.index(i, j, 0);
            const Vec3 center = field[c];
            const Vec3 xp = field[grid.index(i + 1, j, 0)];
            const Vec3 xm = field[grid.index(i - 1, j, 0)];
            const Vec3 yp = field[grid.index(i, j + 1, 0)];
            const Vec3 ym = field[grid.index(i, j - 1, 0)];
            const Vec3 lap_x = multiply(add(add(xp, xm), multiply(center, -2.0)),
                                        1.0 / (dx.x * dx.x));
            const Vec3 lap_y = multiply(add(add(yp, ym), multiply(center, -2.0)),
                                        1.0 / (dx.y * dx.y));
            result[c] = add(lap_x, lap_y);
        }
    }
    return result;
}

}  // namespace boris
