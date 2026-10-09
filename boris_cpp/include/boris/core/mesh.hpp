#pragma once

#include <array>
#include <cmath>
#include <stdexcept>
#include "boris/core/boundary.hpp"
#include "boris/core/vector.hpp"

// Minimal axis-aligned uniform mesh with spatially constant E and B fields.
namespace boris {

class UniformMesh {
public:
    UniformMesh(Vec3 origin, Vec3 extent, std::array<int, 3> cells,
                Vec3 electric_field = {}, Vec3 magnetic_field = {},
                BoundaryCondition boundary = BoundaryCondition::Absorbing)
        : origin_(origin), extent_(extent), cells_(cells), electric_field_(electric_field),
          magnetic_field_(magnetic_field), boundary_(boundary) {
        if (!(extent.x > 0.0 && extent.y > 0.0 && extent.z > 0.0) ||
            cells[0] <= 0 || cells[1] <= 0 || cells[2] <= 0) {
            throw std::invalid_argument("mesh extents and cell counts must be positive");
        }
    }

    bool contains(Vec3 position) const {
        return position.x >= origin_.x && position.x < origin_.x + extent_.x &&
               position.y >= origin_.y && position.y < origin_.y + extent_.y &&
               position.z >= origin_.z && position.z < origin_.z + extent_.z;
    }
    Vec3 electric_field(Vec3 /*position*/) const { return electric_field_; }
    Vec3 magnetic_field(Vec3 /*position*/) const { return magnetic_field_; }
    Vec3 origin() const { return origin_; }
    Vec3 extent() const { return extent_; }
    std::array<int, 3> cells() const { return cells_; }
    BoundaryCondition boundary() const { return boundary_; }

private:
    Vec3 origin_;
    Vec3 extent_;
    std::array<int, 3> cells_;
    Vec3 electric_field_;
    Vec3 magnetic_field_;
    BoundaryCondition boundary_;
};
};
