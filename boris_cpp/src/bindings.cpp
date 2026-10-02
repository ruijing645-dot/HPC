#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "boris/boris.hpp"

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
        .def("contains", &boris::UniformMesh::contains);

    module.def("push", &boris::push, py::arg("particle"), py::arg("dt"), py::arg("mesh"));
    module.def("norm", &boris::norm);
}
