"""Simple checks for every PeriodicGrid function and 2D3V particle motion."""

import os
import sys

module_dir = os.environ.get("BORIS_MODULE_DIR")
if module_dir is None:
    module_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
sys.path.insert(0, module_dir)

import boris_cpp as boris


origin = boris.Vec3(1.0, 2.0, 7.0)
extent = boris.Vec3(4.0, 6.0, 0.0)
grid = boris.PeriodicGrid(origin, extent, (4, 3, 8))

# 2D 网格在 z 方向只有一层，共有 4 * 3 个点。
assert tuple(grid.cells) == (4, 3, 1)
assert grid.size() == 12

# spacing() 返回 x、y 的网格间距；z 方向没有网格间距。
spacing = grid.spacing()
assert (spacing.x, spacing.y, spacing.z) == (1.0, 2.0, 0.0)

# wrap_index() 把超出范围的索引周期性绕回。
assert grid.wrap_index(-1, 4) == 3
assert grid.wrap_index(4, 4) == 0

# index() 将二维索引转换为一维数组位置；z 索引会映射到唯一平面。
assert grid.index(-1, 3, 8) == 9

# wrap_position() 只包装 x、y，保留 z 坐标。
point = grid.wrap_position(boris.Vec3(-1.0, 9.0, 123.0))
assert (point.x, point.y, point.z) == (3.0, 3.0, 123.0)

# 2D3V 粒子可以有 z 速度，但 z 位置不变。
mesh = boris.UniformMesh(
    boris.Vec3(0, 0, 0),
    boris.Vec3(4, 4, 0),
    (4, 4, 1),
    boris.Vec3(0, 0, 1),
    boris.Vec3(0, 0, 0),
)
particle = boris.Particle(boris.Vec3(1, 1, 9), boris.Vec3(0, 0, 0), 1, 1)
boris.push(particle, 0.1, mesh)
assert abs(particle.velocity.z - 0.1) < 1e-12
assert particle.position.z == 9

print("test_02_grid_field_push passed")
