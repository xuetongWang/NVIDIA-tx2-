TX2 运行环境配置
================
* [软件环境说明](#软件环境说明)
* [启动流程](#启动流程)
* [内核编译](#内核编译)
    * [编译过程](#编译过程)
    * [说明](#说明)
* [内核配置参数说明](#配置参数说明)
    * [内核调试](#内核调试)
    * [设备树](#设备树)
* [驱动移植](#驱动移植)
* [安装qt](#安装qt)
* [安装ace](#安装ace)
* [安装rabbitmq](#安装rabbitmq)
* [数据库移植](#数据库移植)
* [烧写](#烧写)
* [系统环境变量杂项](#系统环境变量杂项)
    * [挂载 nfs](#挂载)
    * [切换 python](#切换)
    * [音视频解码](音视频解码)
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

## 配置参数说明
### 内核调试
配置内核KGDB如图所示：  
![kgdb](http://192.168.0.2:8089/wangxuetong/Jetpack4.2-tx2/raw/master/docs/images/kgdb.png)
配置完成后，链接串口到minicom，现在开发板状态链接UART1口，板子的硬件链路为J21。详细链接方法参考硬件原理图。有两种内核调试方法：  
（1）在启动过程中调试内核  
（2）启动完成后调试内核  
启动过程调试内核:  
配置/pro/cmdline中的内容，cmdline中的内容来源为uboot中的cbootargs参数传递给到TX2的内核/boot/extlinux/extlinux.conf。extlinux.conf生成cmdline的内容。想修改cmdline的内容去修改extlinux.conf的内容即可。添加kgdb的内容在APPEND中console=ttyS0,115200n8 kgdboc=ttyS0,115200,kgdbwait。  
修改前参数：
```c
root=/dev/mmcblk0p1 rw rootwait console=ttyS0,115200n8 console=tty0 OS=l4t fbcon=map:0 net.ifnames=0 memtype=0 video=tegrafb no_console_suspend=1 earlycon=uart8250,mmio32,0x03100000 nvdumper_reserved=0x2772e0000 gpt tegra_fbmem2=0x800000@0x969ee000 lut_m
em2=0x2008@0x969eb000 tegraid=18.1.2.0.0 tegra_keep_boot_clocks maxcpus=6 boot.slot_suffix= boot.ratchetvalues=0.2.1 androidboot.serialno=0423318020525 bl_prof_dataptr=0x10000@0x277040000 sdhci_tegra.en_boot_part_access=1
```
修改后参数：
```c
root=/dev/mmcblk0p1 rw rootwait console=ttyS0,115200n8 kgdboc=ttyS0,115200 kgdbwait console=tty0 OS=l4t fbcon=map:0 net.ifnames=0 memtype=0 video=tegrafb no_console_suspend=1 earlycon=uart8250,mmio32,0x03100000 nvdumper_reserved=0x2772e0000 gpt tegra_fbmem2=0x800000@0x969ee000 lut_mem2=0x2008@0x969eb000 tegraid=18.1.2.0.0 tegra_keep_boot_clocks maxcpus=6 boot.slot_suffix= boot.ratchetvalues=0.2.1 androidboot.serialno=0423318020525 bl_prof_dataptr=0x10000@0x277040000 sdhci_tegra.en_boot_part_access=1
```
修改/usr/local/jetson-gdb/gdb-7.11/gdb/features/aarch64-core.xml中的<reg name="cpsr" bitsize="64"/>  
启动gdb  
```c
 aarch64-linux-gnu-gdb /home/tinytong/git_pro/runtime/kernel/kernel/kernel-4.4/image/vmlinux
//进入到gdb后配置
set serial baud 115200
set tdesc filename /usr/local/jetson-gdb/gdb-7.11/gdb/features/aarch64.xml
target remote /dev/ttyUSB0
```
进入到调试状态，运行continue即可调试内核。  
运行过程调试内核
在TX2端
```c
echo ttyS0,115200 > /sys/module/kgdboc/parameters/kgdboc

echo -n g > /proc/sysrq-trigger
```
主机端的做法与启动过程中调试内核做法一致。  

### 设备树
根据jetson-tx2.conf可知，设备树主要文件为tegra186-quill-p3310-1000-a00-00-base.dtb。  
内存配置
```c
	memory@80000000 {
		device_type = "memory";
		reg = <0x0 0x80000000 0x0 0x70000000>;
	};
```
tx2 的cpu的物理内存起始地址0x80000000。大小为0x70000000的地址  

## 驱动移植
详细内容参考 [driver](Git_doc/driver.md)

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
安装mysql
```c
apt-get install mysql-server-5.5
apt-get install mysql-client-core-5.5
apt-get install libqt5sql5-mysql
```
mysql密码：sleton
### 程序移植
参考qwt官网编译qwt库文件和qwtpolar文件添加库文件和头文件

### 开机自动登录QT程序
修改 /usr/share/xgreeters/.desktop文件 将application 参数的unity-greeter改成自己的QT程序

## 安装ace
进入github官网wget http://github.com/DOCGroup/ACE_TAO/releases 找到6.4.0源码进行下载
设置环境变量 打开.bashrc文件
在文件末尾添加如下内容
```c
ACE_ROOT=.../ACE_wrappers
export ACE_ROOT=...//这个自己根据目录进行设置
export LD_LIBRARY_PATH=...//这个自己根据目录进行设置
export INSTALL_PREFIX=...//这个自己根据目录进行设置
```
进入源码目录，准备编译文件 创建config.h文件vim ace/config.h
```c
#include "config-linux.h"
```
创建GNU文件vim include/makeinclude/platform_macros.GNU
```c
include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU
```
编译安装make && make install
## 安装tslib
安装tslib 的github网站上的顺序make就行了。添加库文件的时候制定到目录下就可以
## 安装rabbitmq
从https://github.com/alanxz/rabbitmq-c下载源码 进入源码目录：
```c
mkdir build && cd build
cmake ..
cmake --build . --config Release
```
然后把库文件和头文件包含到ARM程序中
## 数据库移植
配置/etc/mysql/my.cnf文件
```c
[mysqld]
skip-grant-tables=1
```

## 烧写
烧写主要应用文件[`flash.sh`](runtime/flash/flash.sh)关于脚本内容说明如下：  
```c
./flash.sh [options] ${target_board} ${target_rootdev}
```
target_board为jetson-tx2 它接受jetson-tx2.conf中的参数。两者在同一目录下。target_rootdev为mmcblk0p或者eth0等。根据不同的启动设备设置不同的即可

主要烧写步骤如下：  
按下并松开TX2的POWER键S4；按下并保持FORCE RECOVERY键S3；在此期间按下并松开RESET键S1；等待2秒后松开FORCE RECOVERY键S3。此时进入到烧写模式  
```c
#烧写uboot和kernel 
sudo ./flash.sh -r -k kernel jetson-tx2 mmcblk0p1
#烧写dtb
sudo ./flash.sh -r -k kernel-dtb jetson-tx2 mmcblk0p1

#更新所有文件uboot kernel rootfs的命令（重新编译rootfs）
./flash.sh jetson-tx2 mmcblk0p1

#更新所有文件uboot kernel rootfs的命令（不重新编译rootfs）-r的作用使用原有文件系统system.img
./flash.sh -r jetson-tx2 mmcblk0p1
#解释伪代码如下：
if [ -r]; then
	APP_TAG+="-e s/filename=system.img/filename=${localsysfile}/ ";
	APP_TAG+="-e s/APPFILE/${localsysfile}/ ";
	echo "Skip generating ${localsysfile}";
	fi;
elif  then
	APP_TAG+="-e s/filename=system.img/filename=${localsysfile}/ ";
	APP_TAG+="-e s/APPFILE/${localsysfile}/ ";
	if [ "${target_partname}" = "" -o "${target_partname}" = "APP" ]; then
		build_fsimg system.img;

#克隆现有主机的image：
./flash.sh -r -k APP -G clone.img jetson-tx2 mmcblk0p1

cp clone.img bootloader/system.img
```
生成的clone.img.raw可以挂载在主机目录下，以便进行文件系统的修改  
```c
#挂载
mount -o loop clone.img.raw /mnt
#修改完成取消挂载
umount /mnt
#生成image
cp clone.img.raw /where/ever/Linux_for_Tegra/bootloader/system.img
```
## 系统环境变量杂项
### 挂载

```c
sudo mount 192.168.1.101:/home/root /mnt
```
### 切换
```c
sudo update-alternatives --config python
```

### 数据库硬盘挂载

* 查看硬盘信息
```c
sudo blkid
```

* 格式化硬盘为ext4 格式
```c
mkfs.ext4 /dev/硬盘盘符
```

* 挂载到media/sleton下面
```c
sudo vi /etc/fstab
/dev/sda    /meida/sleton   ext4  defaults     1   2 #置0为不备份，置1，2为备份，但2的备份重要性比1小    置0为不检验，置1，2为检验，但置2盘比置1的盘晚检验。
```

### 音视频解码
音视频解码主要应用nvidia的硬件解码加速器，具体内容参考[videodecode](Git_doc/videodecode.md)

## 参考文档

* L4T使用说明：[L4T使用文档](https://docs.nvidia.com/jetson/l4t/index.html)
