#pragma once

#include <cmath>

#include "boris/core/vector.hpp"

namespace boris {

enum class BoundaryCondition { Absorbing, Periodic, Reflecting };

inline double wrap_coordinate(double value, double low, double length) {
    double offset = std::fmod(value - low, length);
    if (offset < 0.0) offset += length;
    return low + offset;
}

inline Vec3 wrap_position(Vec3 position, Vec3 origin, Vec3 extent) {
    return {wrap_coordinate(position.x, origin.x, extent.x),
            wrap_coordinate(position.y, origin.y, extent.y),
            wrap_coordinate(position.z, origin.z, extent.z)};
}

inline void apply_particle_boundary(Vec3& position, Vec3& velocity, Vec3 origin,
                                    Vec3 extent, BoundaryCondition boundary, bool& alive) {
    if (position.x >= origin.x && position.x < origin.x + extent.x &&
        position.y >= origin.y && position.y < origin.y + extent.y &&
        position.z >= origin.z && position.z < origin.z + extent.z) {
        return;
    }

    if (boundary == BoundaryCondition::Absorbing) {
        alive = false;
    } else if (boundary == BoundaryCondition::Periodic) {
        position = wrap_position(position, origin, extent);
    } else {
        auto reflect = [](double& coordinate, double& component_velocity, double low,
                          double length) {
            double offset = std::fmod(coordinate - low, 2.0 * length);
            if (offset < 0.0) offset += 2.0 * length;
            if (offset >= length) {
                coordinate = low + 2.0 * length - offset;
                component_velocity = -component_velocity;
            } else {
                coordinate = low + offset;
            }
        };
        reflect(position.x, velocity.x, origin.x, extent.x);
        reflect(position.y, velocity.y, origin.y, extent.y);
        reflect(position.z, velocity.z, origin.z, extent.z);
    }
}

}  // namespace boris
