视频处理
==========================
* [环境版本信息](#环境版本信息)
* [主机端软件安装](#主机端软件安装)
* [主机训练](#主机训练)
* [音视频解码](#音视频解码)
* [并行编程部分](#并行编程部分)
* [推理优化](#推理优化)
    * [构建模型网络步骤](#构建网络步骤)
* [参考文献](#参考文献)

##  环境版本信息  
* CUDA: 10.0  
* 计算能力 6.1  

## 主机端软件安装：
在官网下载sdkmanager ,打开软件后,选择烧写TX2的AI部分即可,因为软件继承sdk环境已经由四利通软件工作人员烧写,修改完成.

## 主机训练
主机训练详细步骤参考:[TX2 train](Git_doc/train.md)  
<font color=red>注:这里只是为了编写TensorRt网络才进行的训练尝试,与真正的算法工程师进行的网络训练是两码事,这里主要目的是获得一份pb文件</font>

### pb文件操作
* 查看pb文件的tensor张量图可直接替换[botensorboard](Neural_network/train/pbtensorboard.py)文件里面的pb名称，生成log文件.
* 用tensorboard插件查看网络张量图

```c
tensorboard --logdir=path/to/log-directory
```
* TX2挂载主机home目录下的work目录

```c
sudo mount 192.168.1.100:/home/tinytong/work /mnt
```
<font color=red>将获取到的pb文件考入到work文件夹下操作,好处是省的来回来去的拷贝文件,生成文件等等一系列麻烦的操作.</font>
* 进入到TX2的推理项目文件夹中转换.pb文件为uff格式

```c
convert-to-uff tensorflow --input-file faster_mask_resnet50_frozen_inference_graph.pb -l
```
* 为查看最后一层网络名称 收集到网络名称后执行

```c
convert-to-uff tensorflow -o faster_mask_resnet50_.uff --input-file faster_mask_resnet50_frozen_inference_graph.pb -O  name_of_output_tensor
```
* 例如在ssd网络中我们可以采用config.py 直接

```c
convert-to-uff tensorflow --input-file frozen_inference_graph.pb -O NMS -p config.py
```

## 音视频解码
详细内容参考 [音视频解码](Git_doc/videodecode.md)

## 并行编程部分
* 并行编程部分主要需要了解TX2的硬件架构和CUDA编程思想。具体参考 [CUDA编程](Git_doc/cuda.md)

## 推理优化
### 推理优化步骤
* 创建一个定义模型的TensorRT的网络
* 导入TensorRT的构建器，创建一个优化引擎
* 序列化和反序列化引擎
* 向引擎填入数据进行解析

### 创建一个定义模型的TensorRT的网络(使用C++接口)
* 创建 IBuilder优化网络
* 创建 IRuntime执行优化网络

##### 创建网络
创建网络有两种方式 最简单的办法就是使用TENSORRT解析器库，第二种方法就是直接使用TENSORRT的接口。在这两种情况下，您都需要显式地告诉TensorRT哪些张量是推理输出所必需的。没有标记为输出的张量可能被构建器优化掉的瞬时值。输出张量的数量没有限制，但是将张量标记为输出可能会禁止对该张量进行某些优化。还必须为输入和输出张量指定名称(使用ITensor::setName())。在推理时，为引擎提供一个指向输入和输出缓冲区的指针数组。为了确定引擎期望这些指针的顺序，可以使用张量名称进行查询。**项目里使用解析器库构建网络**

1.创建构建器和网络  
2.创建解析器  
3.使用解析器解析模型输出网络  

##### 使用tensorflow网络(uff解析器)
1.创建构建器和网络

```python
IBuilder* builder = createInferBuilder(gLogger);
INetworkDefinition* network = builder->createNetwork();
```

2.创建解析器

```python
IUFFParser* parser = createUffParser();
```

3.声明UFF解析器的网络输入和输出

```python
parser->registerInput("Input_0", DimsCHW(1, 28, 28), UffInputOrder::kNCHW);
parser->registerOutput("Binary_3");
```

4.使用解析器解析模型输出网络

```python
parser->parse(uffFile, *network, nvinfer1::DataType::kFLOAT);
```

### 导入TensorRT的构建器，创建一个优化引擎
1.使用builder 创建引擎

```python
builder->setMaxBatchSize(maxBatchSize);
builder->setMaxWorkspaceSize(1 << 20);
ICudaEngine* engine = builder->buildCudaEngine(*network);
```

2.如果使用的是网络本身(详情参考构建网络的两种方式)，则不适用builder，parser和网络
### 序列化和反序列化引擎
要进行序列化，您需要将引擎转换为一种格式，以便在以后的推理中存储和使用。要用于推理，只需反序列化引擎即可。序列化和反序列化是可选的。由于从网络定义创建引擎可能会耗费时间，因此您可以避免在每次应用程序重新运行时重新构建引擎，方法是对引擎进行一次序列化，并在进行推断时反序列化引擎。因此，在构建引擎之后，用户通常希望序列化它以供以后使用。  

1.序列化
```python
IHostMemory *serializedModel = engine->serialize();
// store model to disk
// <…>
serializedModel->destroy();
```

2.反序列化

```python
IRuntime* runtime = createInferRuntime(gLogger);
ICudaEngine* engine = runtime->deserializeCudaEngine(modelData, modelSize, nullptr);
```

### 向引擎填入数据进行解析

1.创建一些空间来存储中间激活值。由于引擎包含网络定义和训练参数，因此需要额外的空间。它们在执行上下文中保存，使用输入和输出blob名称来获得相应的输入和输出索引

```python
IExecutionContext *context = engine->createExecutionContext();
int inputIndex = engine.getBindingIndex(INPUT_BLOB_NAME);
int outputIndex = engine.getBindingIndex(OUTPUT_BLOB_NAME);
```
2.使用这些索引，设置一个缓冲区数组指向GPU上的输入和输出缓冲区

```python
void* buffers[2];
buffers[inputIndex] = inputbuffer;
buffers[outputIndex] = outputBuffer;
```

3.TensorRT的执行通常是异步的，因此将内核排队到CUDA流中

```python
context.enqueue(batchSize, buffers, stream, nullptr);
```

## 参考文献
[Tensorflow官方网站](https://tensorflow.google.cn/guide/summaries_and_tensorboard)  
[预训练模型下载地址](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/detection_model_zoo.md)

