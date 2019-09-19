#!/bin/bash

echo "当前操作目录 $PWD"
export CROSS_COMPILE=/usr/local/arm64gcc/gcc-linaro-7.3.1-2018.05-i686_aarch64-linux-gnu/bin/aarch64-linux-gnu-

TEGRA_KERNEL_OUT=/home/tinytong/work/github/tx2/Sdk_platform

MODULE_KERNEL_OUT=${TEGRA_KERNEL_OUT}/Linux_for_Tegra/rootfs
TOP_IMAGE=${TEGRA_KERNEL_OUT}/Kernel_4.9_Src/image
MAKE_SRC=${PWD}


########################################################

cd ${MAKE_SRC}

mv .config ${TOP_IMAGE}

if [ ! -f ${TOP_IMAGE}/.config ];then
  echo "config 失败==检查源文件目录是否存在配置文件"
  exit
fi

if [ "$1" = "module" ];then
   echo "make module"
   make ARCH=arm64 O=$TOP_IMAGE modules_install INSTALL_MOD_PATH=$MODULE_KERNEL_OUT
elif [ "$1" = "dtb" ];then 
   echo "make dtbs "
   make ARCH=arm64 O=$TOP_IMAGE dtbs
   mv ${TOP_IMAGE}/arch/arm64/boot/dts/tegra186-quill-p3310-1000-c03-00-base.dtb ${TEGRA_KERNEL_OUT}/Linux_for_Tegra/kernel/dtb
else
   echo "make kernel -j"
   make ARCH=arm64 O=$TOP_IMAGE -j4
fi
