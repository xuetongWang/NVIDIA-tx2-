TX2 运行环境配置
================
* [软件环境说明](#软件环境说明)
* [启动流程](#启动流程)
* [内核编译](#内核编译)
    * [编译过程](#编译过程)
    * [说明](#说明)
* [驱动移植](#驱动移植)
* [安装qt](#安装qt)
* [烧写](#烧写)
* [参考文档](#参考文档)

## 软件环境说明
* Ubuntu版本：18.04  
* linux内核版本：4.9.140-tegra  
* board/module和revision号：p2771-0000  
* 板级支持包：tegra-l4t-r32.1  
* dtb根目录：tegra186-quill-p3310-1000-c03-00-base.dtb  

## 内核编译
* 编译过程  
进入/home/tinytong/Tx2newPro/Sdk_platform/Kernel_4.9_Src目录下并在目录下新建image文件夹  
在/home/tinytong/work/github/tx2/Sdk_platform/Kernel_4.9_Src/kernel/kernel-4.9下执行：

```c
make ARCH=arm64 tegra_defconfig
```

配置内核编译选项：

```c
make ARCH=arm64 menuconfig
```
进行配置。配置完成之后执行[build_kernel.sh](../Sdk_platform/Kernel_4.9_Src/kernel/kernel-4.9/build_kernel.sh)即可构建内核  

修改内核的时候把.config 文件重新从image拷贝到kernel-4.9后重新编译即可。
* 说明  
build_kernel.sh为编译脚本，但是前提是有配置文件，第一次编译的时候是没有配置文件的。所以第一次需要用tegra_defconfig生成.config文件。生成了.config之后最好自己保存一份常用的.config文件，平时执行menuconfig配置就行了。  
## 驱动移植
详细内容参考 [driver](../Git_doc/driver.md)

## 安装QT
### 安装
打开所有镜像源
```c
sudo apt-get install qt5-default qtcreator -y
```
配置编译器
```c
Tools->Options->Build & Run->Compilers
custom – arm – linux – generic – elf – 64 bit
```
## 烧写
烧写主要应用文件[flash.sh](../Sdk_platform/Linux_for_Tegra/flash.sh)关于脚本内容说明如下：  
```c
./flash.sh [options] ${target_board} ${target_rootdev}
```
target_board为jetson-tx2 它接受jetson-tx2.conf中的参数。两者在同一目录下。target_rootdev为mmcblk0p或者eth0等。根据不同的启动设备设置不同的即可

主要烧写步骤如下：  
按下并松开TX2的POWER键S4；按下并保持FORCE RECOVERY键S3；在此期间按下并松开RESET键S1；等待2秒后松开FORCE RECOVERY键S3。此时进入到烧写模式  
* 烧写uboot和kernel 
```c
sudo ./flash.sh -r -k kernel jetson-tx2 mmcblk0p1(不起什么作用,替换内核的时候直接进入系统，用编译生成的Image文件替换TX2的/boot下的Image文件即可)
```
* 烧写dtb
```c
sudo ./flash.sh -r -k kernel-dtb jetson-tx2 mmcblk0p1
```
* 更新所有文件uboot kernel rootfs的命令（重新编译rootfs）
```c
./flash.sh jetson-tx2 mmcblk0p1
```
* 更新所有文件uboot kernel rootfs的命令（不重新编译rootfs）-r的作用使用原有文件系统system.img
```c
./flash.sh -r jetson-tx2 mmcblk0p1
```
* 克隆现有主机的image：
```c
./flash.sh -r -k APP -G clone.img jetson-tx2 mmcblk0p1

cp clone.img bootloader/system.img
```
生成的clone.img.raw可以挂载在主机目录下，以便进行文件系统的修改  
* 挂载
```c
mount -o loop clone.img.raw /mnt
```
* 修改完成取消挂载
```c
umount /mnt
```
* 生成image
```c
cp clone.img.raw /where/ever/Linux_for_Tegra/bootloader/system.img
```
### 音视频解码
音视频解码主要应用nvidia的硬件解码加速器，具体内容参考[videodecode](Git_doc/videodecode.md)

## 参考文档

* [L4T使用文档](https://docs.nvidia.com/jetson/l4t/index.html)
