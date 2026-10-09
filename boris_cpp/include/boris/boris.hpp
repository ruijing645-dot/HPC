#pragma once

#include <array>
#include <cmath>
#include <stdexcept>

namespace boris {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline Vec3 add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 subtract(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 multiply(Vec3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }
inline double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double norm(Vec3 a) { return std::sqrt(dot(a, a)); }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

enum class BoundaryCondition { Absorbing, Periodic, Reflecting };

// Minimal axis-aligned uniform mesh with spatially constant E and B fields.
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

struct Particle {
    Vec3 position;
    Vec3 velocity;
    double charge = 1.0;
    double mass = 1.0;
    bool alive = true;
};

inline void push(Particle& particle, double dt, Vec3 electric, Vec3 magnetic) {
    if (!(particle.mass > 0.0) || !(dt > 0.0)) {
        throw std::invalid_argument("particle mass and timestep must be positive");
    }
    const double half_qdt_over_m = particle.charge * dt / (2.0 * particle.mass);
    const Vec3 v_minus = add(particle.velocity, multiply(electric, half_qdt_over_m));
    const Vec3 t = multiply(magnetic, half_qdt_over_m);
    const Vec3 v_prime = add(v_minus, cross(v_minus, t));
    const Vec3 s = multiply(t, 2.0 / (1.0 + dot(t, t)));
    const Vec3 v_plus = add(v_minus, cross(v_prime, s));
    particle.velocity = add(v_plus, multiply(electric, half_qdt_over_m));
}

// Fields are sampled at the predicted half-step position, as in BORIS_PUSHER.ipynb.
inline void push(Particle& particle, double dt, const UniformMesh& mesh) {
    if (!particle.alive) return;
    if (!(particle.mass > 0.0) || !(dt > 0.0)) {
        throw std::invalid_argument("particle mass and timestep must be positive");
    }

    const Vec3 half_position = add(particle.position, multiply(particle.velocity, 0.5 * dt));
    const Vec3 electric = mesh.electric_field(half_position);
    const Vec3 magnetic = mesh.magnetic_field(half_position);
    push(particle, dt, electric, magnetic);
    particle.position = add(half_position, multiply(particle.velocity, dt));

    const Vec3 origin = mesh.origin();
    const Vec3 extent = mesh.extent();
    if (mesh.contains(particle.position)) return;

    if (mesh.boundary() == BoundaryCondition::Absorbing) {
        particle.alive = false;
    } else if (mesh.boundary() == BoundaryCondition::Periodic) {
        auto wrap = [](double value, double low, double length) {
            double offset = std::fmod(value - low, length);
            if (offset < 0.0) offset += length;
            return low + offset;
        };
        particle.position = {wrap(particle.position.x, origin.x, extent.x),
                             wrap(particle.position.y, origin.y, extent.y),
                             wrap(particle.position.z, origin.z, extent.z)};
    } else {
        // Reflect at each crossed face; repeated folding handles large overshoots.
        auto reflect = [](double& position, double& velocity, double low, double length) {
            double offset = std::fmod(position - low, 2.0 * length);
            if (offset < 0.0) offset += 2.0 * length;
            if (offset >= length) {
                position = low + 2.0 * length - offset;
                velocity = -velocity;
            } else {
                position = low + offset;
            }
        };
        reflect(particle.position.x, particle.velocity.x, origin.x, extent.x);
        reflect(particle.position.y, particle.velocity.y, origin.y, extent.y);
        reflect(particle.position.z, particle.velocity.z, origin.z, extent.z);
    }
}

}  // namespace boris
