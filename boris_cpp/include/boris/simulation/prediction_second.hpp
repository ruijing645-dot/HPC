#pragma once

#include "boris/simulation/prediction_first.hpp"

namespace boris {

// Second prediction starts from the first prediction's fields and particles.
// It currently uses the same single-step operation as the first pass.
inline PredictionResult predict_second(const PeriodicGrid& grid, const PredictionInput& input,
                                       const PredictionResult& first, double dt) {
    PredictionInput second_input = input;
    second_input.electric = first.electric_half;
    second_input.magnetic = first.magnetic_next;
    second_input.particles = first.particles_next;
    return predict_first(grid, second_input, dt);
}

}  // namespace boris
