### 向量与粒子 vector.hpp-> 网络 mesh.hpp -> 网络插值 -> 预测/校正 -> python 接口于测试
### Core folder:

    1. <mark>vector.hpp</mark>:
    #pragma once 则避免同一个编译单元里重复处理该头文件
    namespace boris{} 在这个大函数里的定义
    struct Vec3 ->  Vec3里面的数据结构
    incline 中定义 add; substrct; multiply; dot; norm; cross
    ## 语法： inline 返回类型 函数名(参数列表) { 函数体}
    2. mesh.hpp: - basic mesh constant EB
        UniformMesh(输入值)：成员初始化列表 (它把传入的参数放进对象对应的成员变量) {检查网格长度和格子数是否为正，不符合要求就抛出异常}
        bool constains - 函数名 也是在写函数    是不是在函数内
        ## 成员函数的定义 在class里不需要 incline： Vec3 ... 如果外面引这个定义就 能在这
        ## 语法： class 类名 {
            public:
                // 外部可以访问的成员
            private:
                // 只有类自身的成员函数可以直接访问
            };
        ## 使用时： UniformMesh mesh(origin, extent, cells, electric, magnetic); 这行代码创建了一个 UniformMesh 类型的对象，并把它命名为 mesh
    3. grid_utils.hpp:
        - 全项目采用 2D3V：网格位置只在 x-y 平面变化，粒子速度和电磁场仍保留 x、y、z 三个分量
        - 保存网格信息：origin 和 extent 的 x、y 分量描述二维平面；cells[2] 会固定为 1
        - size() 返回二维网格节点总数 Nx × Ny；spacing() 的 z 分量为 0
        - index(i, j, k) 把二维网格坐标映射为一维索引，k 总是映射到唯一的 z 平面
        - wrap_position(position) 只周期包装 x、y，z 坐标保持不变
    4. boundary.hpp:
       wrap_coordinate(value, low, length) 定义这个算法
       wrap_position(position, origin, extent) 只包装 x、y 坐标
       apply_particle_boundary 定义二维平面内的粒子边界
### mechanisms:

    1. boris.hpp:
    struct Prticle{位置，速度，电荷，质量，alive} 粒子状态
    push(输入) {检查正} ：两种push
    先用电场做半步速度更新，再通过磁场旋转速度，最后再用电场做半步更新
        1. push(Particle&, double, Vec3 electric, Vec3 magnetic) 
            接收电场和磁场，直接用 Boris 推进公式更新粒子的速度。它不更新位置，也不处理边界    
        2. push(Particle& particle, double dt, const UniformMesh&mesh)
            接收网格，会先根据粒子位置从网格采样电场和磁场，再调用上面的版本更新速度；之后更新位置，并处理吸收、周期或反射边界
    ## const 表示这些中间结果在定义后不再重新赋值
    2. cic.hpp: CIC（Cloud-In-Cell，云团在网格）插值：在粒子位置附近找出网格单元，并计算各单元对该位置的权重。它本身不推进粒子，也不保存电磁场
     周围四个网格点的权重；二维 cell 的四个角点
    struct CicPointWeight{} 变量
    cic_weights
    1. grid.wrap_position(position) 先将位置周期性映射回网格范围。
    2. grid.spacing() 取得 x、y 两个方向的网格间距
    3. 将物理位置换算为以网格间距为单位的坐标
       const int i0 = static_cast<int>(std::floor(gx));
       这行先用 std::floor(gx) 向下取整，再用 static_cast<int> 把结果转换成整数，赋给 i0。i0 表示 x 方向上，粒子所在位置左侧（或正好所在）的网格节点索引
    4. const double fx = gx - i0;
        计算粒子在节点 i0 和下一个节点 i0 + 1 之间的位置比例。比如 gx = 2.3 时，fx = 0.3，x 方向上两个节点的权重分别是 0.7 和 0.3
        for ...定义 wx 和 wy，两个方向的权重相乘得到四个角点的权重。
    interpolate_cic
        values 是调用者传进来的网格数据数组，通常是每个节点上的场值
        ### 语法 ： CicPointWeight item = weights[i];        // 复制一份
                    const CicPointWeight& item = weights[i]; // 引用原对象，只读
                    CicPointWeight& item = weights[i];       // 引用原对象，可修改
6. simulation folder:
    - simulation_types.hpp：保存 prediction 和 correction 共用的输入、输出结构
    - field_operators.hpp：实现二维周期网格上的 curl 和 Laplacian；向量仍有 x、y、z 三个分量，空间导数满足 d/dz=0
    - faraday.hpp：单独实现法拉第方程 B(t+dt)=B(t)-dt*curl(E)
    - prediction_first.hpp、prediction_second.hpp：分别提供第一次、第二次预测入口
    - correction.hpp：实现磁场更新和电场校正，并调用共享的 Faraday、curl 和 Laplacian 函数
    - prediction.hpp：兼容入口，包含上面各头文件；新代码也可以直接包含需要的具体文件

7. 测试 notebook：
    - test_02_grid_field_push.ipynb 检查 PeriodicGrid 的每个函数
    - test_03 至 test_06 分别检查 CIC、两次 prediction 和 correction
------
Python 接口
bindings.cpp： 把C++类型和函数暴露给python
grid_utls的加入后：test pusher
