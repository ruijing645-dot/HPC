#pragma once

namespace boris {

// Dimensionless model parameters; adjust these for the chosen simulation.
struct SimulationConstants {
    double electron_temperature = 1.0;
    double resistivity = 0.0;
    double viscosity = 0.0;
};

inline constexpr SimulationConstants default_constants{};

}  // namespace boris
