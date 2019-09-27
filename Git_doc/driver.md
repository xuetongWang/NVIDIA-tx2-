驱动
================
* [PCIE卡驱动修改](#PCIE卡驱动修改)
* [两路HDMI显示](#两路HDMI显示)
* [总线CAN配置](#总线CAN配置)
* [串口触摸屏](#串口触摸屏)

## PCIE卡驱动修改 
根据datasheet所知，TX2的PCIE共支持5线,3个接口，可配置  
* X4+X1
* X2 +X1 +X1
* X1 +X1 +X1

PCIE接口如下所示：  
* 0接口: PEX2(LINE2)------PEX_RFU(LINE3)------PEX0(LINE0)------USB_SS1(LINE1):(PEX0_REFCLK)
* 1接口：PEX2(LINE0)------------------------------------:(PEX2_REFCLK)
* 2接口：PEX1(LINE0)------------------------------------:(PEX1_REFCLK)
可配置选项如图
![PCIE1](https://github.com/xuetongWang/NVIDIA-tx2-/raw/master/Git_doc/images/1.png)
任何X4的总线可以单独使用X2线(LINE0&LINE1)或者可以使用X1线(LINE0).任何使用X2总线可以使用X1线(LINE0)

* 由于内核已经集成了RTL8168的网卡驱动，所以我们不必更改网卡驱动部分的内容，只是吧module改成集成到内核之中，因为我们一直要使用小网卡，只是编译成模块没有意义。

* 修改PCIE的usb map
根据我们的电路图,应该配置成如图所示的MAP中的3配置  
配置完成后通过ls /sys/firmware/devicetree/base/chosen/plugin-manager/odm-data | grep uphy命令查看line的映射情况  
按照[USB MAP LINE](https://elinux.org/Jetson/TX2_USB)进行配置

配置主要涉及的文件有
[`tegra186-quill-p3310-1000-a00-plugin-manager.dtsi`](Sdk_platform/Kernel_4.9_Src/hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-plugin-manager/tegra186-quill-p3310-1000-a00-plugin-manager.dtsi)  
[`tegra186-quill-p3310-1000-a00-00-base.dts`](Sdk_platform/Kernel_4.9_Src/hardware/nvidia/platform/t18x/quill/kernel-dts/tegra186-quill-p3310-1000-a00-00-base.dts)  
[`tegra186-quill-p3310-1000-c03-00-base.dts`](Sdk_platform/Kernel_4.9_Src/hardware/nvidia/platform/t18x/quill/kernel-dts/tegra186-quill-p3310-1000-c03-00-base.dts)

### 修改tca9539
由于ttyusb触摸屏需要tca9539的管脚3进行供电，所以需要配置tca9539的设备树信息
打开/home/tinytong/git_pro/runtime/kernel/hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-platforms的tegra186-cvb-prod-p2597-b00-p3310-1000-a00-00文件进行配置。修改如下
```c
gpio@74 {
	touch-rails {
		gpio-hog;
		gpios = <1 0 2 0 3 0>;
		output-high;
		label = "touch-rail-1", "touch-rail-2", "ttyusb-power";
	};
};

en_avdd_disp_3v3: regulator@8 {
	compatible = "regulator-fixed-sync";
	reg = <8>;
	regulator-name = "en-vdd-disp-3v3";
	regulator-min-microvolt = <3300000>;
	regulator-max-microvolt = <3300000>;
	gpio = <&gpio_i2c_0_77 0 0>;
	enable-active-high;
};
```
执行cat /sys/kernel/debug/gpio 可以查看gpio的映射情况.
然后执行./build_kernel.sh dtb 。建立好后烧写dtb到tx2即可
### 小网卡与4G模块
修改p2771-0000.conf.common里面的ODMDATA配置。依据  
![ODMDATA](http://192.168.0.2:8089/wangxuetong/Jetpack4.2-tx2/raw/master/docs/images/ODMA.png)  


## 两路HDMI显示

## 总线CAN配置
* 内核配置打开CAN---MTTCAN----CAN-RAW
* 切换到TX2终端或者写进脚本 ip link set up can0 type can bitrate (波特率) dbitrate (采样频率) berr-reporting on fd on
* ip link set up can0
## 串口触摸屏
* 供电芯片TCA9539 主要修改[`tegra186-cvb-prod-p2597-b00-p3310-1000-a00-00.dtsi`](Sdk_platform/Kernel_4.9_Src/hardware/nvidia/platform/t18x/common/kernel-dts/tegra186-cvb-prod-p2597-b00-p3310-1000-a00-00.dtsi)
 
## 数据库硬盘挂载

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







