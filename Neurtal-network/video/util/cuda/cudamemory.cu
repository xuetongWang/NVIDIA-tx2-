#include "cudamemory.h"
#include "cudaUtility.h"

bool cudaAllocMapped( void** cpuPtr, void** gpuPtr, size_t size )
{
	if( !cpuPtr || !gpuPtr || size == 0 )
		return false;

        if( CUDA_FAILED(cudaHostAlloc(cpuPtr, size, cudaHostAllocMapped)) )
		return false;

	if( CUDA_FAILED(cudaHostGetDevicePointer(gpuPtr, *cpuPtr, 0)) )
		return false;

	memset(*cpuPtr, 0, size);
	printf("[cuda]  cudaAllocMapped %zu bytes, CPU %p GPU %p\n", size, *cpuPtr, *gpuPtr);
	return true;
}
