CUDA编程
================================

## pascal架构
pascal架构的SM单元如图所示：

![pascal architecture one sm](http://192.168.0.2:8089/wangxuetong/newServerproject/raw/master/Git_doc/images/20180325130354195.png)  

* 包含了256个核心 最大操作频率 1.12GHZ
* 一个SM分成了4个部分(SMP)，每个SMP包含包含自己的缓冲器，warp调度器和32个CUDA核。
* TX2的CUDA计算能力为6.2

## CUDA_Occupancy_Calculator配置使用
/usr/local/cuda-10.0/tools下面包含了CUDA_Occupancy_Calculator的execl文件，在CUDA编程的过程配置  

* 通过getdeviceprop获得TX2属性如下：
```c
---------device iNFO num = 0----
name = NVIDIA Tegra X2---- 
compute capability:6.2----计算能力6.2
colock rate::1020000---- 工作频率
------device copy overlap:----
Enabled 支持cudamemory和核函数同时调用
Kernel execition timeout : Disabled 核函数不限制运行时
---------Memory iNFO for device = 0----
total global mem : 8233357312---- 全局内存7.66GB
total const mem : 65536----  常量内存64KB
max  mem pitch : 2147483647---- 内存复制最大修正2GB
Texture  Alignment : 512---- 设备文理对齐512
---------Mp iNFO for device = 0----
Multiprocessor count  : 2----
Shared mem per  mp : 49152---- 块共享内存48KB
Registers pre mp  : 32768---- 块共享寄存器32KB
Threads in warp  : 32--- warp 线程数 32
Max threads  pre block  : 1024---- 块最大线程数1024
Max threads  dimensions  : (1024, 1024, 64) 每个线程块最大线程
Max grid  dimensions  : (2147483647, 65535, 65535) 每个GRID最大线程块
```
## 参考文献

[CUDA计算能力](https://developer.nvidia.com/cuda-gpus)


