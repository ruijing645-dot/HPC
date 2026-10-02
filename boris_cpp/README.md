### test note
1. can retrigure the density  expected density/calculate density  -> has corrilation 
2. trojectory - lamor radius 
3. test amdahl

write expaction//

### note for construction
1. 可以把1d mesh 拆成两个process 什么的来计算
2. or initial 分成不同process 但是最后的contribution都得算上 

### note from session 4
1. bug 得确定是不是真的在parappll的情况下
2. for racecondition .. vectorization和CPU threads 只需要知道里面的概念
3. data parallelism 大家干同一件事； task parallelism 大家干不同的事情
4. shell 中输入top 正在运行的所有process
5. 怎么实际创建这些线程 -> 1. OpenMP/OpenACC  你告诉编译器“这里可以并行”，编译器帮你生成多线程代码;OpenMP simple to write, hard to optimize
2.Thread libraries:不依赖编译器自动帮你划分，而是程序员自己显式创建和控制 threads; C++ standard threads -std::thread; pthread-这是 Unix/Linux 上很底层、很经典的线程 API;TBB = Intel Threading Building Blocks - 你不一定自己控制“thread 0 做什么”，而是提交 tasks，让 runtime 自动调度
6. Amdahl's law (important)   if increase the problem -> not suitable match with increase in node
7. Gustafson's law 

pessf
最后展示gprof  optimization 