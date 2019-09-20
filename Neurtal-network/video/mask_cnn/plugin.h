#pragma once

#include "NvInferPlugin.h"
#include "NvUffParser.h"
#include <NvInfer.h>


class ssdPluginFactory : public nvinfer1::IPluginFactory, public nvuffparser::IPluginFactory{

public:
    nvinfer1::IPluginV2* createPlugin(const char* layerName, const nvinfer1::Weights* weights, int nbWeights, const nvuffparser::FieldCollection fc) override;

};
