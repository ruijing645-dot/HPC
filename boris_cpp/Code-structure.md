### 向量与粒子 vector.hpp-> 网络 mesh.hpp -> 网络插值 -> 预测/校正 -> python 接口于测试
### Core folder:

    1. <mark>vector.hpp</mark>:
    #pragma once 则避免同一个编译单元里重复处理该头文件
    namespace boris{} 在这个大函数里的定义
    struct Vec3 ->  Vec3里面的数据结构
    incline 中定义 add; substrct; multiply; dot; norm; cross
    ## 语法： inline 返回类型 函数名(参数列表) { 函数体}
    2. <mark>mesh.hpp</mark>: - basic mesh constant EB
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
    4. boundary.hpp:
       wrap_coordinate(value, low, length) 定义这个算法
       wrap_position(position, origin, extent) 对x y z 都使用
       apply_particle_boundary
### mechanisms:

    1. <mark>boris.hpp</mark>:
    struct Prticle{位置，速度，电荷，质量，alive} 粒子状态
    push(输入) {检查正} ：两种push
    先用电场做半步速度更新，再通过磁场旋转速度，最后再用电场做半步更新
        1. push(Particle&, double, Vec3 electric, Vec3 magnetic) 
            接收电场和磁场，直接用 Boris 推进公式更新粒子的速度。它不更新位置，也不处理边界    
        2. push(Particle& particle, double dt, const UniformMesh&mesh)
            接收网格，会先根据粒子位置从网格采样电场和磁场，再调用上面的版本更新速度；之后更新位置，并处理吸收、周期或反射边界
    ## const 表示这些中间结果在定义后不再重新赋值

6. prediction.hpp:

7. test_boris.hpp:
------
Python 接口
bindings.cpp： 把C++类型和函数暴露给python
