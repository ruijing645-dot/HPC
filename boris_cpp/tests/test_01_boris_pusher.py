"""Smoke tests for the Boris particle pusher."""

import math
import os
import sys
import unittest

sys.path.insert(0, os.environ["BORIS_MODULE_DIR"])
import boris_cpp as boris


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


class BorisPusherTests(unittest.TestCase):
    def test_magnetic_rotation_preserves_speed(self):
        particle = boris.Particle(
            boris.Vec3(1.0, 1.0, 1.0), boris.Vec3(0.0, 1.0, 0.0), 1.0, 0.1
        )
        initial_speed = boris.norm(particle.velocity)
        boris.push(particle, 0.01, make_mesh(magnetic=boris.Vec3(0.0, 0.0, 1.0)))
        self.assertTrue(math.isclose(boris.norm(particle.velocity), initial_speed, rel_tol=1e-12))

    def test_uniform_electric_acceleration(self):
        particle = boris.Particle(
            boris.Vec3(5.0, 5.0, 5.0), boris.Vec3(0.0, 0.0, 0.0), 1.0, 2.0
        )
        boris.push(particle, 0.1, make_mesh(electric=boris.Vec3(2.0, 0.0, 0.0)))
        self.assertAlmostEqual(particle.velocity.x, 0.1, places=12)
        self.assertAlmostEqual(particle.position.x, 5.01, places=12)

    def test_periodic_boundary(self):
        particle = boris.Particle(
            boris.Vec3(9.9, 5.0, 5.0), boris.Vec3(10.0, 0.0, 0.0), 1.0, 1.0
        )
        boris.push(particle, 0.1, make_mesh(boris.BoundaryCondition.PERIODIC))
        self.assertTrue(particle.alive)
        self.assertAlmostEqual(particle.position.x, 1.4, places=12)


if __name__ == "__main__":
    unittest.main()
