#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void HandleError( cudaError_t err,
                         const char *file,
                         int line ) {
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", cudaGetErrorString( err ),
                file, line );
        exit( EXIT_FAILURE );
    }
}
#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))

void mog_malloc_gpu(int32_t device_id, 
				int32_t page_size, 
				char** pages,
				int32_t page_id) {
	cudaSetDevice(device_id);
	char *page;
	HANDLE_ERROR(cudaMalloc((void **)&page, page_size));
	pages[page_id] = page;
}

void mog_memcpy_cpu_to_gpu(int32_t device_id, char* dst, const char* src, int32_t page_size) {
	cudaSetDevice(device_id);
	HANDLE_ERROR(cudaMemcpy(dst, src, page_size, cudaMemcpyHostToDevice));
	//cudaDeviceSynchronize();
}

void mog_memcpy_gpu_to_cpu(int32_t device_id, char* dst, const char* src, int32_t page_size) {
	cudaSetDevice(device_id);
	HANDLE_ERROR(cudaMemcpy(dst, src, page_size, cudaMemcpyDeviceToHost));
	//cudaDeviceSynchronize();
}

/*
void mog_vectorAdd(int32_t device_id, const char *A, const char *B, char *C, int numElements) {
	cudaError_t err = cudaSuccess;
	
	cudaSetDevice(device_id);
	int threadsPerBlock = 256;
    int blocksPerGrid =(1024*1024 + threadsPerBlock - 1) / threadsPerBlock;
    
	vectorAdd<<<blocksPerGrid, threadsPerBlock>>>(A, B, C, numElements);
	
	err = cudaGetLastError();
	if (err != cudaSuccess) {
        fprintf(stderr, "Failed to kernel function (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}
*/
