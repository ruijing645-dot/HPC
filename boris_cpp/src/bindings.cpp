#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "boris/mechanisms/boris.hpp"
#include "boris/simulation/prediction.hpp"

namespace py = pybind11;

PYBIND11_MODULE(boris_cpp, module) {
    module.doc() = "Boris particle pusher with a uniform mesh";

    py::class_<boris::Vec3>(module, "Vec3")
        .def(py::init<double, double, double>())
        .def_readwrite("x", &boris::Vec3::x)
        .def_readwrite("y", &boris::Vec3::y)
        .def_readwrite("z", &boris::Vec3::z);

    py::enum_<boris::BoundaryCondition>(module, "BoundaryCondition")
        .value("ABSORBING", boris::BoundaryCondition::Absorbing)
        .value("PERIODIC", boris::BoundaryCondition::Periodic)
        .value("REFLECTING", boris::BoundaryCondition::Reflecting);

    py::class_<boris::Particle>(module, "Particle")
        .def(py::init<boris::Vec3, boris::Vec3, double, double>())
        .def_readwrite("position", &boris::Particle::position)
        .def_readwrite("velocity", &boris::Particle::velocity)
        .def_readwrite("charge", &boris::Particle::charge)
        .def_readwrite("mass", &boris::Particle::mass)
        .def_readwrite("alive", &boris::Particle::alive);

    py::class_<boris::UniformMesh>(module, "UniformMesh")
        .def(py::init<boris::Vec3, boris::Vec3, std::array<int, 3>, boris::Vec3, boris::Vec3,
                      boris::BoundaryCondition>(),
             py::arg("origin"), py::arg("extent"), py::arg("cells"),
             py::arg("electric_field"), py::arg("magnetic_field"),
             py::arg("boundary") = boris::BoundaryCondition::Absorbing)
         .def("contains", &boris::UniformMesh::contains)
         .def("electric_field", &boris::UniformMesh::electric_field)
         .def("magnetic_field", &boris::UniformMesh::magnetic_field)
         .def_property_readonly("origin", &boris::UniformMesh::origin)
         .def_property_readonly("extent", &boris::UniformMesh::extent)
         .def_property_readonly("cells", &boris::UniformMesh::cells);

    module.def("push",
               static_cast<void (*)(boris::Particle&, double, const boris::UniformMesh&)>(&boris::push),
               py::arg("particle"), py::arg("dt"), py::arg("mesh"));
    module.def("push",
               static_cast<void (*)(boris::Particle&, double, boris::Vec3, boris::Vec3)>(&boris::push),
               py::arg("particle"), py::arg("dt"), py::arg("electric"), py::arg("magnetic"));
    module.def("norm", &boris::norm);

    py::class_<boris::PeriodicGrid>(module, "PeriodicGrid")
        .def(py::init<boris::Vec3, boris::Vec3, std::array<int, 3>>(), py::arg("origin"),
             py::arg("extent"), py::arg("cells"));
    py::class_<boris::PredictionInput>(module, "PredictionInput")
        .def(py::init<>())
        .def_readwrite("electric", &boris::PredictionInput::electric)
        .def_readwrite("magnetic", &boris::PredictionInput::magnetic)
        .def_readwrite("particles", &boris::PredictionInput::particles);
    py::class_<boris::PredictionResult>(module, "PredictionResult")
        .def_readonly("magnetic_next", &boris::PredictionResult::magnetic_next)
        .def_readonly("electric_half", &boris::PredictionResult::electric_half)
        .def_readonly("particles_next", &boris::PredictionResult::particles_next)
        .def_readonly("density_next", &boris::PredictionResult::density_next)
        .def_readonly("bulk_velocity_next", &boris::PredictionResult::bulk_velocity_next);
    module.def("predict_first", &boris::predict_first, py::arg("grid"), py::arg("input"), py::arg("dt"));
    module.def("predict_second", &boris::predict_second, py::arg("grid"), py::arg("input"),
               py::arg("first"), py::arg("dt"));
    py::class_<boris::CorrectionInput>(module, "CorrectionInput")
        .def(py::init<>())
        .def_readwrite("magnetic", &boris::CorrectionInput::magnetic)
        .def_readwrite("electric_half", &boris::CorrectionInput::electric_half)
        .def_readwrite("density", &boris::CorrectionInput::density)
        .def_readwrite("bulk_velocity", &boris::CorrectionInput::bulk_velocity)
        .def_readwrite("electron_pressure", &boris::CorrectionInput::electron_pressure);
    py::class_<boris::CorrectionResult>(module, "CorrectionResult")
        .def_readonly("magnetic", &boris::CorrectionResult::magnetic)
        .def_readonly("electric", &boris::CorrectionResult::electric);
    module.def("correct_fields", &boris::correct_fields, py::arg("grid"), py::arg("input"),
               py::arg("dt"), py::arg("eta"), py::arg("nu"));
}
