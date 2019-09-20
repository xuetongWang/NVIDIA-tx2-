#ifndef __CUDA_MEMORY_H__
#define __CUDA_MEMORY_H__

#include "cudaUtility.h"

bool cudaAllocMapped( void** cpuPtr, void** gpuPtr, size_t size );

#endif
