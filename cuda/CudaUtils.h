#pragma once

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

// zpracování chyb
// error handling
inline void handle_error(cudaError_t err, const char* file, int line) {
    if (err != cudaSuccess) {
        printf("%s in %s at line %d\n", cudaGetErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}

#define HANDLE_ERROR(err) (handle_error(err, __FILE__, __LINE__))