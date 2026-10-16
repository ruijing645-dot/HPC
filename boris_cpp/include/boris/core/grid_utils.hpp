#pragma once

#include <array>
#include <stdexcept>
#include <vector>

#include "boris/core/boundary.hpp"
#include "boris/core/vector.hpp"

namespace boris {

// Node-centered, periodic, uniform 2-D grid utilities for 2D3V simulations.
// Fields and particle velocities have three components, but positions are
// represented only in x and y. The z axis stores one plane.
struct PeriodicGrid {
    Vec3 origin;
    Vec3 extent;
    std::array<int, 3> cells;

    PeriodicGrid(Vec3 grid_origin, Vec3 grid_extent, std::array<int, 3> grid_cells)
        : origin(grid_origin), extent(grid_extent), cells(grid_cells) {
        if (!(extent.x > 0.0 && extent.y > 0.0) || cells[0] <= 0 || cells[1] <= 0) {
            throw std::invalid_argument("2-D grid x/y extents and cell counts must be positive");
        }
        cells[2] = 1;
    }

    std::size_t size() const {
        return static_cast<std::size_t>(cells[0]) * cells[1] * cells[2];
    }

    Vec3 spacing() const {
        return {extent.x / cells[0], extent.y / cells[1], 0.0};
    }

    int wrap_index(int index, int count) const {
        int result = index % count;
        return result < 0 ? result + count : result;
    }

    std::size_t index(int i, int j, int k) const {
        const int wi = wrap_index(i, cells[0]);
        const int wj = wrap_index(j, cells[1]);
        const int wk = wrap_index(k, cells[2]);
        return (static_cast<std::size_t>(wi) * cells[1] + wj) * cells[2] + wk;
    }

    Vec3 wrap_position(Vec3 position) const {
        position.x = boris::wrap_coordinate(position.x, origin.x, extent.x);
        position.y = boris::wrap_coordinate(position.y, origin.y, extent.y);
        return position;
    }
};

}  // namespace boris
