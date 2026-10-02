import os
import sys
import unittest

sys.path.insert(0, os.environ["BORIS_MODULE_DIR"])
import boris_cpp as boris


def mesh(boundary=boris.BoundaryCondition.ABSORBING, electric=None, magnetic=None):
    return boris.UniformMesh(
        boris.Vec3(0.0, 0.0, 0.0), boris.Vec3(10.0, 10.0, 10.0), (10, 10, 10),
        electric or boris.Vec3(0.0, 0.0, 0.0), magnetic or boris.Vec3(0.0, 0.0, 0.0), boundary,
    )


class BorisTests(unittest.TestCase):
    def test_magnetic_field_preserves_speed(self):
        particle = boris.Particle(boris.Vec3(1.0, 1.0, 1.0), boris.Vec3(0.0, 1.0, 0.0), 1.0, 0.1)
        boris.push(particle, 0.01, mesh(magnetic=boris.Vec3(0.0, 0.0, 1.0)))
        self.assertAlmostEqual(boris.norm(particle.velocity), 1.0, places=12)

    def test_periodic_boundary_wraps_particle(self):
        particle = boris.Particle(boris.Vec3(9.9, 5.0, 5.0), boris.Vec3(10.0, 0.0, 0.0), 1.0, 1.0)
        boris.push(particle, 0.1, mesh(boris.BoundaryCondition.PERIODIC))
        self.assertTrue(particle.alive)
        self.assertAlmostEqual(particle.position.x, 1.4)

    def test_reflecting_boundary_reverses_normal_velocity(self):
        particle = boris.Particle(boris.Vec3(9.9, 5.0, 5.0), boris.Vec3(10.0, 0.0, 0.0), 1.0, 1.0)
        boris.push(particle, 0.1, mesh(boris.BoundaryCondition.REFLECTING))
        self.assertTrue(particle.alive)
        self.assertAlmostEqual(particle.position.x, 8.6)
        self.assertAlmostEqual(particle.velocity.x, -10.0)


if __name__ == "__main__":
    unittest.main()
