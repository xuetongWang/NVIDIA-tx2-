TX2 音视频解码过程
================
* [驱动应用手册](#驱动应用手册)
* [解码](#解码)
* [相机捕获](#相机捕获)
* [视频缩放](#视频缩放)
* [工程说明](#工程说明)

## 驱动应用手册
版本：GSTREAMER-1.0  

本版本包括一下gst-omx视频解码器：  
  
![decode](http://192.168.0.2:8089/wangxuetong/Tx2/raw/master/docs/images/decode.png)  
  
  
本版本包括一下gst-omx视频接受器：  
![receive](http://192.168.0.2:8089/wangxuetong/Tx2/raw/master/docs/images/receive.png)  
本版本包括一下英伟达插件：  
![receive](http://192.168.0.2:8089/wangxuetong/Tx2/raw/master/docs/images/videoplugin.png)  
## 解码
以下示例显示如何在GSTREAMER-1.0上执行视频解码(使用gst-launch-1.0)  
H.264解码（使用英伟达硬件加速也就是NVDEC）
```c
gst-launch-1.0 filesrc location=<filename.mp4> ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! omxh264dec ! nveglglessink -e
```
H.265解码
```c
gst-launch-1.0 filesrc location=<filename.mp4> ! qtdemux name=demux demux.video_0 ! queue ! h265parse ! omxh265dec ! nvoverlaysink -e
```

## 相机捕获
项目工程中nvgstcapture-1.0应用程序使用了rtsp来捕获静态图像和视频。
```c
gst-launch-1.0 rtsprc location="<ip/port>"  ! queue ! h264parse ! omxh264dec ! nveglglessink -e
```
## 视频缩放
NVIDIA专有的nvvidconv Gstreamer-1.0插件执行视频缩放，支持I420, UYVY, YUY2, YVYU, NV12, GRAY8, BGRx,RGBA。
```c
gst-launch-1.0 filesrc location=1280x720_30p.mp4 ! qtdemux ! queue ! h264parse ! omxh264dec ! nvvidconv ! 'video/x-raw, format=(string)I420, width=640, height=480' !  xvimagesink -e
```

## 工程说明
整体流程为RTSP视频流====>gstreame管道解码接收======>NV12图像转RGBA图像======>神经网络

### 管道
```c
"rtspsrc location=rtsp://admin:123456@192.168.1.108/cam/realmonitor?channel=1&subtype=1 latency=0 ! decodebin ! nvvidconv ! video/x-raw,format=(string)RGBA,width=(int), height=(int) mHeightfFramerate=30/1 ! appsink name=mysink ";
```
### 内存存储
对于4*4的图片，NV12的存储方式如下：
```c
	Y1  Y2  Y3  Y4
	Y5  Y6  Y7  Y8
	Y9  Y10 Y11 Y12
	Y13 Y14 Y15 Y16
	U1  V1  U2  V2
	U3  V3  U4  V4

```
内存中的存储如下
```c
Y1Y2Y3Y4Y5Y6Y7Y8Y9Y10Y11Y12Y13Y14Y15Y16U1V1U2V2U3V3U4V4
```
具体的采样方式可自行查询.Y”表示明亮度（Luminance、Luma），“U”和“V”则是色度与浓度（Chrominance、Chroma。可见每次收取的数据size为width*height*1.5。  

对于4*4的图片，RGBA的存储方式如下：
```c
	RGBA1    RGBA2   RGBA3   RGBA4
	RGBA5    RGBA6   RGBA7   RGBA8
	RGBA9    RGBA10  RGBA11  RGBA12
	RGBA13   RGBA14  RGBA15  RGBA16
```
内存中存储如下
```c
RGBA1RGBA2RGBA3RGBA4RGBA5RGBA6RGBA7RGBA8RGBA9RGBA10RGBA11RGBA12RGBA13RGBA14RGBA15RGBA16
```
最后转换成RRRRGGGGBBB的形式，采用cuda代码，详情请看工程[`cudaNormalize.sh`](inference/Videoinference/Camerstream/cuda/cudaNormalize.cu)

