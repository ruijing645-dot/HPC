"""Small runnable smoke test for the compiled boris_cpp Python module.

Build the module first with ``./build.sh``, then run ``python test_pusher.py``.
"""

import math
import sys
from pathlib import Path


BUILD_DIR = Path(__file__).resolve().parent / "build"
sys.path.insert(0, str(BUILD_DIR))

import boris_cpp as boris  # noqa: E402


def make_mesh(boundary=boris.BoundaryCondition.ABSORBING, electric=None, magnetic=None):
    zero = boris.Vec3(0.0, 0.0, 0.0)
    return boris.UniformMesh(
        boris.Vec3(0.0, 0.0, 0.0),
        boris.Vec3(10.0, 10.0, 10.0),
        (10, 10, 10),
        electric if electric is not None else zero,
        magnetic if magnetic is not None else zero,
        boundary,
    )


def test_magnetic_rotation_preserves_speed():
    particle = boris.Particle(
        boris.Vec3(1.0, 1.0, 1.0), boris.Vec3(0.0, 1.0, 0.0), 1.0, 0.1
    )
    initial_speed = boris.norm(particle.velocity)
    boris.push(particle, 0.01, make_mesh(magnetic=boris.Vec3(0.0, 0.0, 1.0)))
    assert math.isclose(boris.norm(particle.velocity), initial_speed, rel_tol=1e-12)
    print("PASS: magnetic field preserves speed")


def test_uniform_electric_acceleration():
    particle = boris.Particle(
        boris.Vec3(5.0, 5.0, 5.0), boris.Vec3(0.0, 0.0, 0.0), 1.0, 2.0
    )
    dt = 0.1
    boris.push(
        particle,
        dt,
        make_mesh(electric=boris.Vec3(2.0, 0.0, 0.0)),
    )
    assert math.isclose(particle.velocity.x, 0.1, abs_tol=1e-12)
    assert math.isclose(particle.position.x, 5.01, abs_tol=1e-12)
    print("PASS: uniform electric field updates velocity and position")


def test_periodic_boundary():
    particle = boris.Particle(
        boris.Vec3(9.9, 5.0, 5.0), boris.Vec3(10.0, 0.0, 0.0), 1.0, 1.0
    )
    boris.push(particle, 0.1, make_mesh(boris.BoundaryCondition.PERIODIC))
    assert particle.alive
    assert math.isclose(particle.position.x, 1.4, abs_tol=1e-12)
    print("PASS: periodic boundary wraps the particle")


def main():
    test_magnetic_rotation_preserves_speed()
    test_uniform_electric_acceleration()
    test_periodic_boundary()
    print("All Boris pusher tests passed.")


if __name__ == "__main__":
    main()
