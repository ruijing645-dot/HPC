#pragma once

#include <array>
#include <stdexcept>
#include "boris/core/boundary.hpp"
#include "boris/core/mesh.hpp"

namespace boris {
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

    apply_particle_boundary(particle.position, particle.velocity, mesh.origin(), mesh.extent(),
                            mesh.boundary(), particle.alive);
}

}  // namespace boris
