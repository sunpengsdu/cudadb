#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void db_malloc_gpu(int32_t page_size, int32_t page_num, char** pages) {
	cudaSetDevice(0);
	char *page;
	cudaMalloc((void **)&page, 1024*page_size*1024);
	pages[0] = page;
	cudaSetDevice(1);
	cudaMalloc((void **)&page, 1024*page_size*1024);
}
