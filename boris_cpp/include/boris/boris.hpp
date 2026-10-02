#pragma once

#include <array>
#include <cmath>
#include <stdexcept>

namespace boris {

struct Vec3 {
    double x{};
    double y{};
    double z{};

    Vec3& operator+=(const Vec3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
};

inline Vec3 operator+(Vec3 left, const Vec3& right) { return left += right; }
inline Vec3 operator*(Vec3 value, double scalar) {
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}
inline Vec3 operator*(double scalar, Vec3 value) { return value * scalar; }
inline double dot(const Vec3& left, const Vec3& right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}
inline Vec3 cross(const Vec3& left, const Vec3& right) {
    return {left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x};
}
inline double norm(const Vec3& value) { return std::sqrt(dot(value, value)); }

enum class BoundaryCondition { Absorbing, Periodic, Reflecting };

struct Particle {
    Vec3 position;
    Vec3 velocity;
    double charge{1.0};
    double mass{1.0};
    bool alive{true};
};

class UniformMesh {
public:
    UniformMesh(Vec3 origin, Vec3 extent, std::array<int, 3> cells,
                Vec3 electric_field, Vec3 magnetic_field,
                BoundaryCondition boundary = BoundaryCondition::Absorbing)
        : origin_(origin), extent_(extent), cells_(cells), electric_field_(electric_field),
          magnetic_field_(magnetic_field), boundary_(boundary) {
        if (extent.x <= 0.0 || extent.y <= 0.0 || extent.z <= 0.0 ||
            cells[0] <= 0 || cells[1] <= 0 || cells[2] <= 0) {
            throw std::invalid_argument("mesh extent and cell counts must be positive");
        }
    }

    Vec3 electric_field_at(const Vec3&) const { return electric_field_; }
    Vec3 magnetic_field_at(const Vec3&) const { return magnetic_field_; }
    bool contains(const Vec3& point) const {
        return point.x >= origin_.x && point.x < origin_.x + extent_.x &&
               point.y >= origin_.y && point.y < origin_.y + extent_.y &&
               point.z >= origin_.z && point.z < origin_.z + extent_.z;
    }

    void apply_boundary(Particle& particle) const {
        apply_axis(particle.position.x, particle.velocity.x, origin_.x, extent_.x, particle);
        apply_axis(particle.position.y, particle.velocity.y, origin_.y, extent_.y, particle);
        apply_axis(particle.position.z, particle.velocity.z, origin_.z, extent_.z, particle);
    }

    const Vec3& origin() const { return origin_; }
    const Vec3& extent() const { return extent_; }
    const std::array<int, 3>& cells() const { return cells_; }

private:
    void apply_axis(double& position, double& velocity, double lower, double length,
                    Particle& particle) const {
        if (!particle.alive || (position >= lower && position < lower + length)) return;
        if (boundary_ == BoundaryCondition::Absorbing) {
            particle.alive = false;
            return;
        }
        if (boundary_ == BoundaryCondition::Periodic) {
            position = lower + std::fmod(std::fmod(position - lower, length) + length, length);
            return;
        }

        const double period = 2.0 * length;
        double wrapped = std::fmod(std::fmod(position - lower, period) + period, period);
        if (wrapped >= length) {
            position = lower + period - wrapped;
            velocity = -velocity;
        } else {
            position = lower + wrapped;
        }
    }

    Vec3 origin_;
    Vec3 extent_;
    std::array<int, 3> cells_;
    Vec3 electric_field_;
    Vec3 magnetic_field_;
    BoundaryCondition boundary_;
};

// Uses the same midpoint position update as BORIS_PUSHER.ipynb.
inline void push(Particle& particle, double dt, const UniformMesh& mesh) {
    if (!particle.alive) return;
    if (particle.mass <= 0.0) throw std::invalid_argument("particle mass must be positive");

    const Vec3 r_half = particle.position + particle.velocity * (dt * 0.5);
    const Vec3 electric = mesh.electric_field_at(r_half);
    const Vec3 magnetic = mesh.magnetic_field_at(r_half);
    const double half_qdt_over_m = particle.charge * dt / (2.0 * particle.mass);
    const Vec3 v_minus = particle.velocity + electric * half_qdt_over_m;
    const Vec3 t = magnetic * half_qdt_over_m;
    const Vec3 v_prime = v_minus + cross(v_minus, t);
    const Vec3 s = t * (2.0 / (1.0 + dot(t, t)));
    const Vec3 v_plus = v_minus + cross(v_prime, s);

    particle.velocity = v_plus + electric * half_qdt_over_m;
    particle.position = r_half + particle.velocity * dt;
    mesh.apply_boundary(particle);
}

}  // namespace boris
