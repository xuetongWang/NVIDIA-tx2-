训练
================
* [训练流程](#训练流程)
    * [数据准备阶段](#数据准备阶段)
    * [文件修改阶段](#文件修改阶段)
    * [训练过程](#训练过程)
    * [算法优化设置](#算法优化设置)
    * [网络模型设置](#网络模型设置)

## 训练流程
### 数据准备阶段
* 从现场视频中截取单帧图像(通过暴风影音和 F5 快捷键保存到指定目录下);
* 在 Linux 系统中,将所有图片进行重命名,对应的脚本文件为:/home/sleton/data/tensorflow/VOCdevkit/rename_4.py,需要提前修改好路径和保存路径;
* 进 入 到 /home/sleton/software/labelImg-master/ 目 录 中 , 直 接 在 终 端 运 行 :python labelImg.py,设置好输入输出路径,进行标记,最终生成 xml 文件;
* 将 命 名 后 的 图 像 放 置 到 /VOCdevkit/VOC2007/JPEGImages 中 , 将 xml 文 件 放 置 到/VOCdevkit/VOC2007/Annotations 中;
* 进行数据增强,一般情况下只会进行对称镜像和对比度亮度噪点等增强操作;修改如下两个脚本中的读写路径,分别进行增强操作 

```python
/home/sleton/data/tensorflow/VOCdevkit/mirrir.py or noise.py
```
* 重新命名,名字是 6 位数字,从 0 开始,例如:000000.jpg,000001.jpg,...,000154.jpg;需要注意的是,xml 文件的名字与 jpg 文件的名字是对应一致的,换而言之,每一个 jpg 都会有一个xml 文件.
* 修改脚本/home/sleton/data/tensorflow/VOCdevkit/make_main_txt.py 中的读写路径,划分好训练集和验证集;最终的文件路径子目录如下:

```c
VOCdevkit
VOC2007
Annotations
JPEGImages
ImageSets
```
* 生成 tfrecord 文件,对应的运行脚本为:/home/sleton/data/tensorflow/VOCdevkit/create_pascal_tf_record.py直接在终端中输入:

```python
python /home/sleton/data/tensorflow/VOCdevkit/create_pascal_tf_record.py \
--label_map_path=/home/sleton/data/tensorflow/pascal_label_map.pbtxt \
--data_dir=/home/sleton/data/tensorflow/VOCdevkit --year=VOC2007 \
--set=train \
--output_path=/home/sleton/data/tensorflow/VOCdevkit/pascal_train.record
```
然后修改--set=test \

```python
--output_path=/home/sleton/data/tensorflow/VOCdevkit/pascal_test.record
```
生成 test 的训练文件.
将上述文件 copy 至:/home/sleton/data/tensorflow/models/research/object_detection/self/record/
### 文件修改阶段
以 ssd_resnet50_v1_fpn 为例子:/home/sleton/data/tensorflow/models/research/object_detection/self/pipeline/ssd_resnet50_v1_fpn.config
* 修改 num_classes 为自定义数据集中类别的总数量.num_classes: 4
* 设置图片的大小,将图像大小分别输入到对应的 height 和 width.

```python
image_resizer {
fixed_shape_resizer {
height: 540
width: 960
}
}
```
* 设置 batch_size,一般来说,batch_size 设置的越大越好,但越大显存消耗剧增,若是运行过程出现内存不够,就减小该数值,直到内存够用为止.

```python
train_config {
batch_size: 4
data_augmentation_options {
random_horizontal_flip {
}
}
```
* 设置学习率和衰减系数,一般应根据样本量进行设置,每 5 个 epoch 衰减一次:

```python
learning_rate: {
manual_step_learning_rate {
initial_learning_rate: 0.01
schedule {
step: 1
learning_rate: 0.01
}
schedule {
step: 24000
learning_rate: 0.005
}
schedule {
step: 1200000
learning_rate: 0.0003
}
}
}
```
* 设置 tfrecord 路径:
将*.record 存放路径写入如下的位置:pascal_label_map.pbtxt 的内容根据训练对象进行调整.

```python
train_input_reader:
tf_record_input_reader {
input_path:
"/home/sleton/data/tensorflow/models/research/object_detection/self/record/pascal_train.record"
}
label_map_path:
"/home/sleton/data/tensorflow/models/research/object_detection/self/record/pascal_label_map.pbtxt"
}
```
同理将验证集写入下边对应的路径:

```python
eval_input_reader: {
tf_record_input_reader {
input_path:
"/home/sleton/data/tensorflow/models/research/object_detection/self/record/pascal_te
st.record"
- 2 -}
label_map_path:
"/home/sleton/data/tensorflow/models/research/object_detection/self/record/pascal_la
bel_map.pbtxt"
shuffle: false
num_readers: 1
}
```
* 设置 fine_tune_checkpoint 路径,这里存放的是与训练的网络模型,若是应用了现有的模型,需要去官网下载,然后解压放置到指定的位置.
[tensorflow模型网址](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/detection_model_zoo.md)

fine_tune_checkpoint:  
"/home/sleton/data/tensorflow/models/research/object_detection/self/pre_trained/ssd_resnet50_v1_fpn/model.ckpt"from_detection_checkpoint: true
若是自己设计的网络模型,则直接将上述两行注释掉即可,表示从头开始训练.
### 训练过程
```python
python /home/sleton/data/tensorflow/models/research/object_detection/legacy/train.py \
--train_dir=/home/sleton/data/tensorflow/models/research/object_detection/self/results/s
sd_resnet50_v1_fpn \
--pipeline_config_path=/home/sleton/data/tensorflow/models/research/object_detection/sel
f/pipeline/ssd_resnet50_v1_fpn.config
```
直接输入上述命令即可,标黑的需要修改,改成自己指定的名称;
Tensorboard 查看训练过程:
Tensorboard --logdir=
/home/sleton/data/tensorflow/models/research/object_detection/self/results/ssd_densenet_v2
导出*.pb 文件:
对应脚本为:
```python
python /home/sleton/data/tensorflow/utils/export_inference_graph.py
```
设置图中标黑部分:
```python
flags.DEFINE_string('pipeline_config_path',
'/home/sleton/data/tensorflow/models/research/object_detection/self/results/ssd_mobilene
t_v2_fpn/pipeline.config',
'Path to a pipeline_pb2.TrainEvalPipelineConfig config ' 'file.')
flags.DEFINE_string('trained_checkpoint_prefix',
'/home/sleton/data/tensorflow/models/research/object_detection/self/results/ssd_mobilene
t_v2_fpn/model.ckpt', 'Checkpoint prefix.')
flags.DEFINE_string('output_directory',
'/home/sleton/data/tensorflow/models/research/object_detection/self/results/ssd_mobilene
t_v2_fpn/',
'Path to write outputs.')
```
然后直接运行如下脚本:
```python
python /home/sleton/data/tensorflow/utils/export_inference_graph.py \
--config_override " \
model{ \
ssd{ \
post_processing { \
- 3 -batch_non_max_suppression { \
iou_threshold: 0.5 \
} \
} \
} \
} \
"
```
### 算法优化设置
* 根据样本中的目标分布,anchors 设置长宽比和尺度大小:长宽比的获取通过 k_means.py 聚类获得:


```python
/home/sleton/data/tensorflow/models/YOLOv3/get_kmeans.py
anchor_generator {
multiscale_anchor_generator {
min_level: 3
max_level: 7
anchor_scale: 4.0
aspect_ratios: 1.0
aspect_ratios: 2.0
aspect_ratios: 0.5
scales_per_octave: 2
}
}
```
* 修改特征提取网络,这个稍微复杂一下,我在里面集成了一些自己改动的内容:

```python
/home/sleton/data/tensorflow/models/research/slim/nets/resnet_v1.py
feature_extractor {
type: "ssd_resnet50_v1_fpn"
depth_multiplier: 1.0
min_depth: 16
conv_hyperparams {
regularizer {
l2_regularizer {
weight: 0.000399999989895
}
}
```
* 剩余的结合最新论文,去复现论文的算法.

## 网络模型设置
### YOLO网络的设置
* anchor_per_scale:3 表示对每个框进行3种预测


