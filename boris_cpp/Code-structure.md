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
        - 保存网格信息：origin 是网格起点，extent 是三个方向的总长度，cells 是三个方向的单元格数量。构造时会检查长度和单元格数都为正
        - 计算网格大小：size() 返回单元格总数，即 Nx × Ny × Nz
        - 计算单元格间距：spacing() 返回每个方向的网格间距，即 extent / cells
        - 计算线性索引：index(i, j, k) 将三维单元格坐标转换成一维数组索引。越界的单元格坐标会按周期边界循环回网格内，例如 i = -1 会对应最后一个 x 方向单元格
        - 映射物理位置：wrap_position(position) 将超出网格范围的位置周期性映射回网格内；它调用 boundary.hpp 中的公共 wrap_position()，没有重复实现算法
    4. boundary.hpp:
       wrap_coordinate(value, low, length) 定义这个算法
       wrap_position(position, origin, extent) 对x y z 都使用
       apply_particle_boundary 定义具体的边界
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
    周围八个点的权重；3D的 cell 方块的8个点
    struct CicPointWeight{} 变量
    cic_weights
    1. grid.wrap_position(position) 先将位置周期性映射回网格范围。
    2. grid.spacing() 取得三个方向的网格间距
    3. 将物理位置换算为以网格间距为单位的坐标
       const int i0 = static_cast<int>(std::floor(gx));
       这行先用 std::floor(gx) 向下取整，再用 static_cast<int> 把结果转换成整数，赋给 i0。i0 表示 x 方向上，粒子所在位置左侧（或正好所在）的网格节点索引
    4. const double fx = gx - i0;
        计算粒子在节点 i0 和下一个节点 i0 + 1 之间的位置比例。比如 gx = 2.3 时，fx = 0.3，x 方向上两个节点的权重分别是 0.7 和 0.3
        for ...定义 wx，如果 di 非零，就把 fx 赋给它；否则把 1.0 - fx 赋给它。
    interpolate_cic
        values 是调用者传进来的网格数据数组，通常是每个节点上的场值
        ### 语法 ： CicPointWeight item = weights[i];        // 复制一份
                    const CicPointWeight& item = weights[i]; // 引用原对象，只读
                    CicPointWeight& item = weights[i];       // 引用原对象，可修改
6. prediction.hpp:

7. test_boris.hpp:
------
Python 接口
bindings.cpp： 把C++类型和函数暴露给python
grid_utls的加入后：test pusher

