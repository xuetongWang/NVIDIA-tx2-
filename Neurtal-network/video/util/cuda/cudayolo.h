
#ifndef __CUDA_YOLO_H_
#define __CUDA_YOLO_H_

cudaError_t cudaYoloLayerV3(const void* input, void* output, const uint& batchSize,
                            const uint& gridSize, const uint& numOutputClasses,
                            const uint& numBBoxes, uint64_t outputSize, cudaStream_t stream);



#endif
