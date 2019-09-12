NVIDIA TX2的使用总结
================
* [tx2硬件解析](#tx2硬件解析)
* [系统环境搭建](#系统环境搭建)
* [视频处理部分](#视频处理部分)
* [参考文献](#参考文献)

## tx2硬件解析

TX2: Pascal GPU + ARMv8 + 8GB LPDDR4 + 32GB eMMC + WLAN/BT  
* NVIDIA Pascal 图形处理单元（GPU）架构；

* 双核NVIDIA Denver 2 ARM CPU 核心，加四核ARM Cortex-A57核心；

* 基于一个Cortex-A9处理器和独立RAM的独立音频子系统；

* 基于独立的ARM Cortex-R5F 传感器处理器；

* 基于独立的ARM Cortex-R5F 处理器子系统用于处理启动和能量管理；

* 基于独立的ARM Cortex-R5F 处理器子系统用于处理摄像头管理；

* 128位DRAM接口提供了高宽带的LPDDR4支持

* GPU部分为帕斯卡架构，具体内容可参考CUDA编程部分

## 系统环境搭建
详细内容参考 [runtime](Git_doc/runtime.md)
## 视频处理部分
详细内容参考 [TX2 video_inference](Git_doc/video_inference.md)
## 参考文献
[tensorRT使用指导](https://docs.nvidia.com/deeplearning/sdk/tensorrt-archived/tensorrt_401/tensorrt-developer-guide/index.html)  
[cuda使用指导](https://docs.nvidia.com/cuda/index.html)  
[TX2论坛](https://devtalk.nvidia.com/default/board/188/jetson-tx2/)  
[digits使用方法](https://github.com/NVIDIA/DIGITS)  
[tensorRT接口函数文件](https://docs.nvidia.com/deeplearning/sdk/tensorrt-api/c_api/classnvinfer1_1_1_weights.html)  
[cuda Function file](http://www.cs.cmu.edu/afs/cs/academic/class/15668-s11/www/cuda-doc/html/group__CUDART__TYPES_g3f51e3575c2178246db0a94a430e0038.html#g3f51e3575c2178246db0a94a430e0038)  
[tensorRt 支持层](https://docs.nvidia.com/deeplearning/sdk/tensorrt-support-matrix/index.html)  
[JetPack 4.2下载地址](https://developer.nvidia.com/embedded/downloads#?tx=$product,jetson_tx2)  
[L4T 32.1 下载地址](https://developer.nvidia.com/embedded/linux-tegra)  

