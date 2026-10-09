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


class PredictionTests(unittest.TestCase):
    def make_grid(self):
        return boris.PeriodicGrid(boris.Vec3(0.0, 0.0, 0.0), boris.Vec3(4.0, 4.0, 4.0), (4, 4, 4))

    def test_constant_fields_remain_unchanged(self):
        grid = self.make_grid()
        state = boris.PredictionInput()
        state.electric = [boris.Vec3(1.0, -2.0, 0.5) for _ in range(64)]
        state.magnetic = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        result = boris.predict_first(grid, state, 0.1)
        for field in result.magnetic_next:
            self.assertAlmostEqual(field.x, 0.0, places=12)
            self.assertAlmostEqual(field.y, 0.0, places=12)
            self.assertAlmostEqual(field.z, 0.0, places=12)

    def test_periodic_particle_push_and_deposition(self):
        grid = self.make_grid()
        state = boris.PredictionInput()
        state.electric = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.magnetic = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.particles = [boris.Particle(boris.Vec3(3.9, 1.0, 1.0), boris.Vec3(2.0, 0.0, 0.0), 1.0, 1.0)]
        result = boris.predict_first(grid, state, 0.2)
        particle = result.particles_next[0]
        self.assertTrue(particle.alive)
        self.assertAlmostEqual(particle.position.x, 0.3, places=12)
        self.assertAlmostEqual(sum(result.density_next), 1.0, places=12)
        weighted_velocity = sum(result.density_next[i] * result.bulk_velocity_next[i].x for i in range(64))
        self.assertAlmostEqual(weighted_velocity, 2.0, places=12)

    def test_node_centered_field_sampling_at_grid_node(self):
        grid = self.make_grid()
        state = boris.PredictionInput()
        state.electric = [
            boris.Vec3(float(i), float(j), float(k))
            for i in range(4)
            for j in range(4)
            for k in range(4)
        ]
        state.magnetic = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.particles = [
            boris.Particle(boris.Vec3(1.0, 1.0, 1.0), boris.Vec3(0.0, 0.0, 0.0), 1.0, 1.0)
        ]

        result = boris.predict_first(grid, state, 0.1)
        velocity = result.particles_next[0].velocity
        self.assertAlmostEqual(velocity.x, 0.1, places=12)
        self.assertAlmostEqual(velocity.y, 0.1, places=12)
        self.assertAlmostEqual(velocity.z, 0.1, places=12)

    def test_second_prediction_and_correction_keep_uniform_state(self):
        grid = self.make_grid()
        state = boris.PredictionInput()
        state.electric = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.magnetic = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.particles = [boris.Particle(boris.Vec3(1.5, 1.5, 1.5), boris.Vec3(1.0, 0.0, 0.0), 1.0, 1.0)]
        first = boris.predict_first(grid, state, 0.1)
        second = boris.predict_second(grid, state, first, 0.1)
        correction = boris.CorrectionInput()
        correction.magnetic = second.magnetic_next
        correction.electric_half = first.electric_half
        correction.density = [1.0 for _ in range(64)]
        correction.bulk_velocity = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        correction.electron_pressure = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        corrected = boris.correct_fields(grid, correction, 0.1, 0.0, 0.0)
        for field in corrected.electric:
            self.assertAlmostEqual(field.x, 0.0, places=12)
            self.assertAlmostEqual(field.y, 0.0, places=12)
            self.assertAlmostEqual(field.z, 0.0, places=12)

    def test_cic_weights_are_periodic_and_conservative(self):
        grid = self.make_grid()
        state = boris.PredictionInput()
        state.electric = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.magnetic = [boris.Vec3(0.0, 0.0, 0.0) for _ in range(64)]
        state.particles = [boris.Particle(boris.Vec3(3.9, 1.2, 1.3), boris.Vec3(0.0, 0.0, 0.0), 1.0, 1.0)]
        result = boris.predict_first(grid, state, 0.1)
        self.assertAlmostEqual(sum(result.density_next), 1.0, places=12)
        nonzero_cells = sum(1 for density in result.density_next if density > 0.0)
        self.assertEqual(nonzero_cells, 8)


if __name__ == "__main__":
    unittest.main()
