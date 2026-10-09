#pragma once

#include <vector>

#include "boris/cic.hpp"

namespace boris {

struct PredictionInput {
    std::vector<Vec3> electric;
    std::vector<Vec3> magnetic;
    std::vector<Particle> particles;
};

struct PredictionResult {
    std::vector<Vec3> magnetic_next;
    std::vector<Vec3> electric_half;
    std::vector<Particle> particles_next;
    std::vector<double> density_next;
    std::vector<Vec3> bulk_velocity_next;
};

struct CorrectionInput {
    std::vector<Vec3> magnetic;
    std::vector<Vec3> electric_half;
    std::vector<double> density;
    std::vector<Vec3> bulk_velocity;
    std::vector<Vec3> electron_pressure;
};

struct CorrectionResult {
    std::vector<Vec3> magnetic;
    std::vector<Vec3> electric;
};

// First prediction pass: B^{n+1}=B^n-dt curl(E^n), then push particles
// with linearly interpolated periodic fields and deposit number/momentum density.
inline PredictionResult predict_first(const PeriodicGrid& grid, const PredictionInput& input,
                                      double dt) {
    if (!(dt > 0.0)) throw std::invalid_argument("timestep must be positive");
    if (input.electric.size() != grid.size() || input.magnetic.size() != grid.size()) {
        throw std::invalid_argument("electric and magnetic arrays must match grid size");
    }

    PredictionResult result;
    result.magnetic_next = input.magnetic;
    result.electric_half = input.electric;
    result.particles_next = input.particles;
    result.density_next.assign(grid.size(), 0.0);
    result.bulk_velocity_next.assign(grid.size(), Vec3{});
    const Vec3 dx = grid.spacing();

    for (int i = 0; i < grid.cells[0]; ++i) {
        for (int j = 0; j < grid.cells[1]; ++j) {
            for (int k = 0; k < grid.cells[2]; ++k) {
                const std::size_t c = grid.index(i, j, k);
                const Vec3 exp = input.electric[grid.index(i + 1, j, k)];
                const Vec3 exm = input.electric[grid.index(i - 1, j, k)];
                const Vec3 eyp = input.electric[grid.index(i, j + 1, k)];
                const Vec3 eym = input.electric[grid.index(i, j - 1, k)];
                const Vec3 ezp = input.electric[grid.index(i, j, k + 1)];
                const Vec3 ezm = input.electric[grid.index(i, j, k - 1)];
                const Vec3 curl_e{(eyp.z - eym.z) / (2.0 * dx.y) - (ezp.y - ezm.y) / (2.0 * dx.z),
                                  (ezp.x - ezm.x) / (2.0 * dx.z) - (exp.z - exm.z) / (2.0 * dx.x),
                                  (exp.y - exm.y) / (2.0 * dx.x) - (eyp.x - eym.x) / (2.0 * dx.y)};
                result.magnetic_next[c] = subtract(input.magnetic[c], multiply(curl_e, dt));
            }
        }
    }

    for (Particle& particle : result.particles_next) {
        if (!particle.alive) continue;
        const Vec3 e = interpolate_cic(grid, input.electric, particle.position);
        const Vec3 b = interpolate_cic(grid, input.magnetic, particle.position);
        // Existing Boris pusher performs its own half-position field sampling for UniformMesh.
        // Use the direct-field overload here to preserve grid interpolation at the particle.
        push(particle, dt, e, b);
        particle.position = grid.wrap_position(add(particle.position, multiply(particle.velocity, dt)));

        for (const CicPointWeight& item : cic_weights(grid, particle.position)) {
            const double weighted_charge = particle.charge * item.weight;
            result.density_next[item.cell] += weighted_charge;
            result.bulk_velocity_next[item.cell] =
                add(result.bulk_velocity_next[item.cell], multiply(particle.velocity, weighted_charge));
        }
    }

    for (std::size_t cell = 0; cell < grid.size(); ++cell) {
        if (result.density_next[cell] != 0.0) {
            result.bulk_velocity_next[cell] = multiply(result.bulk_velocity_next[cell],
                                                       1.0 / result.density_next[cell]);
        }
    }
    return result;
}

// Second prediction pass using the first pass's half-step electric field.
inline PredictionResult predict_second(const PeriodicGrid& grid, const PredictionInput& input,
                                       const PredictionResult& first, double dt) {
    PredictionInput second_input = input;
    second_input.electric = first.electric_half;
    second_input.magnetic = first.magnetic_next;
    second_input.particles = first.particles_next;
    return predict_first(grid, second_input, dt);
}

inline std::vector<Vec3> curl_periodic(const PeriodicGrid& grid,
                                       const std::vector<Vec3>& field) {
    if (field.size() != grid.size()) throw std::invalid_argument("field array must match grid size");
    const Vec3 dx = grid.spacing();
    std::vector<Vec3> result(grid.size());
    for (int i = 0; i < grid.cells[0]; ++i) {
        for (int j = 0; j < grid.cells[1]; ++j) {
            for (int k = 0; k < grid.cells[2]; ++k) {
                const std::size_t c = grid.index(i, j, k);
                const Vec3 xp = field[grid.index(i + 1, j, k)], xm = field[grid.index(i - 1, j, k)];
                const Vec3 yp = field[grid.index(i, j + 1, k)], ym = field[grid.index(i, j - 1, k)];
                const Vec3 zp = field[grid.index(i, j, k + 1)], zm = field[grid.index(i, j, k - 1)];
                result[c] = {(yp.z - ym.z) / (2.0 * dx.y) - (zp.y - zm.y) / (2.0 * dx.z),
                             (zp.x - zm.x) / (2.0 * dx.z) - (xp.z - xm.z) / (2.0 * dx.x),
                             (xp.y - xm.y) / (2.0 * dx.x) - (yp.x - ym.x) / (2.0 * dx.y)};
            }
        }
    }
    return result;
}

inline std::vector<Vec3> laplacian_periodic(const PeriodicGrid& grid,
                                            const std::vector<Vec3>& field) {
    if (field.size() != grid.size()) throw std::invalid_argument("field array must match grid size");
    const Vec3 dx = grid.spacing();
    std::vector<Vec3> result(grid.size());
    for (int i = 0; i < grid.cells[0]; ++i) {
        for (int j = 0; j < grid.cells[1]; ++j) {
            for (int k = 0; k < grid.cells[2]; ++k) {
                const std::size_t c = grid.index(i, j, k);
                const Vec3 center = field[c];
                const Vec3 xp = field[grid.index(i + 1, j, k)], xm = field[grid.index(i - 1, j, k)];
                const Vec3 yp = field[grid.index(i, j + 1, k)], ym = field[grid.index(i, j - 1, k)];
                const Vec3 zp = field[grid.index(i, j, k + 1)], zm = field[grid.index(i, j, k - 1)];
                result[c] = add(add(multiply(add(add(xp, xm), multiply(center, -2.0)), 1.0 / (dx.x * dx.x)),
                                    multiply(add(add(yp, ym), multiply(center, -2.0)), 1.0 / (dx.y * dx.y))),
                                multiply(add(add(zp, zm), multiply(center, -2.0)), 1.0 / (dx.z * dx.z)));
            }
        }
    }
    return result;
}

// Correction step. electron_pressure is the grid representation of grad(P_e).
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
    const std::vector<Vec3> curl_e = curl_periodic(grid, input.electric_half);
    result.magnetic.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        result.magnetic[i] = subtract(input.magnetic[i], multiply(curl_e[i], dt));
    }
    const std::vector<Vec3> curl_b = curl_periodic(grid, result.magnetic);
    const std::vector<Vec3> lap_b = laplacian_periodic(grid, result.magnetic);
    result.electric.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        if (!(input.density[i] > 0.0)) throw std::invalid_argument("density must be positive for correction");
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
