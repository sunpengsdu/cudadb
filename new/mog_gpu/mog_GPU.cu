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

void mog_malloc_gpu(int64_t device_id, 
				int64_t slab_size, 
				char** slabs,
				int64_t slab_id) {
	cudaSetDevice(device_id);
	char *new_slab;
	HANDLE_ERROR(cudaMalloc((void **)&new_slab, slab_size));
	slabs[slab_id] = new_slab;
}

char* mog_malloc_gpu(int64_t device_id, 
				int64_t size) {
	cudaSetDevice(device_id);
	char *data_p;
	HANDLE_ERROR(cudaMalloc((void **)&data_p, size));
	return data_p;
}

void mog_free_gpu(int64_t device_id, 
				char* data_p) {
	cudaSetDevice(device_id);
	cudaFree ((void*) data_p);
}

void mog_memcpy_cpu_to_gpu(int64_t device_id, char* dst, const char* src, int64_t slabe_size) {
	cudaSetDevice(device_id);
	HANDLE_ERROR(cudaMemcpy(dst, src, slabe_size, cudaMemcpyHostToDevice));
	//cudaDeviceSynchronize();
}

void mog_memcpy_gpu_to_cpu(int64_t device_id, char* dst, const char* src, int64_t slabe_size) {
	cudaSetDevice(device_id);
	HANDLE_ERROR(cudaMemcpy(dst, src, slabe_size, cudaMemcpyDeviceToHost));
	//cudaDeviceSynchronize();
}

void mog_memcpy_gpu_to_gpu(int64_t device_id, char* dst, const char* src, int64_t slabe_size) {
	cudaSetDevice(device_id);
	HANDLE_ERROR(cudaMemcpy(dst, src, slabe_size, cudaMemcpyDeviceToDevice));
	//cudaDeviceSynchronize();
}

/*
void mog_vectorAdd(int64_t device_id, const char *A, const char *B, char *C, int numElements) {
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
