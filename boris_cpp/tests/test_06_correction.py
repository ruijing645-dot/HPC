"""Checks correction, curl, Faraday update, and the 2D Laplacian."""

import os
import sys

module_dir = os.environ.get("BORIS_MODULE_DIR")
if module_dir is None:
    module_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
sys.path.insert(0, module_dir)

import boris_cpp as boris


grid = boris.PeriodicGrid(boris.Vec3(0, 0, 0), boris.Vec3(4, 4, 0), (4, 4, 1))
data = boris.CorrectionInput()
data.magnetic = [boris.Vec3(0, 0, 2) for _ in range(grid.size())]
data.electric_half = [boris.Vec3(0, 0, 0) for _ in range(grid.size())]
data.density = [1.0 for _ in range(grid.size())]
data.bulk_velocity = [boris.Vec3(0, 0, 0) for _ in range(grid.size())]
data.electron_pressure = [boris.Vec3(0, 0, 0) for _ in range(grid.size())]

result = boris.correct_fields(grid, data, 0.1, 0.0, 0.0)
assert len(result.magnetic) == grid.size()
assert len(result.electric) == grid.size()
assert all(b.z == 2 for b in result.magnetic)
assert all(e.x == 0 and e.y == 0 and e.z == 0 for e in result.electric)

# Fz 随 x 变化时，curl(F) 有 y 分量。
field = [boris.Vec3(0, 0, [0, 1, 0, -1][i // 4]) for i in range(grid.size())]
curl = boris.curl_periodic(grid, field)
assert abs(curl[0].y + 1.0) < 1e-12
assert abs(curl[8].y - 1.0) < 1e-12

# 法拉第方程：B_new = B_old - dt * curl(E)。
magnetic_next = boris.advance_magnetic_faraday(
    grid, [boris.Vec3(0, 0, 0) for _ in range(grid.size())], field, 0.1
)
assert abs(magnetic_next[0].y - 0.1) < 1e-12
assert abs(magnetic_next[8].y + 0.1) < 1e-12

# Laplacian 只沿 x、y 方向计算。
lap = boris.laplacian_periodic(grid, field)
assert abs(lap[4].z + 2.0) < 1e-12

print("test_06_correction passed")
