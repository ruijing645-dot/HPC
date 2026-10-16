"""Checks for 2D four-point CIC weights and interpolation."""

import os
import sys

module_dir = os.environ.get("BORIS_MODULE_DIR")
if module_dir is None:
    module_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
sys.path.insert(0, module_dir)

import boris_cpp as boris


grid = boris.PeriodicGrid(boris.Vec3(0, 0, 0), boris.Vec3(4, 4, 0), (4, 4, 1))
point = boris.Vec3(1.25, 2.5, 99)

weights = boris.cic_weights(grid, point)
assert len(weights) == 4
assert abs(sum(item.weight for item in weights) - 1.0) < 1e-12
assert [item.cell for item in weights] == [6, 7, 10, 11]
assert all(item.weight >= 0 for item in weights)

# 网格数据是 (x, y, 1)，线性插值应还原粒子所在位置。
values = [boris.Vec3(i // 4, i % 4, 1) for i in range(grid.size())]
value = boris.interpolate_cic(grid, values, point)
assert abs(value.x - 1.25) < 1e-12
assert abs(value.y - 2.5) < 1e-12
assert abs(value.z - 1.0) < 1e-12

print("test_03_cic passed")
