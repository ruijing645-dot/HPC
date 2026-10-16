"""Checks the first 2D3V prediction step."""

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
data.particles = [boris.Particle(boris.Vec3(1, 1, 7), boris.Vec3(0, 0, 0), 1, 1)]

result = boris.predict_first(grid, data, 0.1)
assert len(result.magnetic_next) == grid.size()
assert all(b.x == 0 and b.y == 0 and b.z == 0 for b in result.magnetic_next)
assert abs(result.particles_next[0].velocity.z - 0.1) < 1e-12
assert result.particles_next[0].position.z == 7
assert abs(sum(result.density_next) - 1.0) < 1e-12

print("test_04_prediction_first passed")
