"""Checks that the second prediction continues from the first result."""

import os
import sys

module_dir = os.environ.get("BORIS_MODULE_DIR")
if module_dir is None:
    module_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
sys.path.insert(0, module_dir)

import boris_cpp as boris


grid = boris.PeriodicGrid(boris.Vec3(0, 0, 0), boris.Vec3(4, 4, 0), (4, 4, 1))
data = boris.PredictionInput()
data.electric = [boris.Vec3(0, 0, 1) for _ in range(grid.size())]
data.magnetic = [boris.Vec3(0, 0, 0) for _ in range(grid.size())]
data.particles = [boris.Particle(boris.Vec3(1, 1, 5), boris.Vec3(0, 0, 0), 1, 1)]

first = boris.predict_first(grid, data, 0.1)
second = boris.predict_second(grid, data, first, 0.1)
particle = second.particles_next[0]
assert abs(particle.velocity.z - 0.2) < 1e-12
assert particle.position.z == 5
assert len(second.density_next) == grid.size()

print("test_05_prediction_second passed")
