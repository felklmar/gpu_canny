/**
 * @file CudaUtils.h
 * @brief Utility functions and macros for robust CUDA error handling.
 */

#pragma once

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Checks a CUDA error code and terminates the program if an error occurred.
 * * @param err The CUDA error code returned by a CUDA API call.
 * @param file The name of the source file where the error occurred (use __FILE__).
 * @param line The line number where the error occurred (use __LINE__).
 */
inline void handle_error(cudaError_t err, const char* file, int line) {
    if (err != cudaSuccess) {
        printf("%s in %s at line %d\n", cudaGetErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}

/**
 * @def HANDLE_ERROR(err)
 * @brief A macro to wrap CUDA API calls for automatic error checking.
 * * Expands to call handle_error with the current file name and line number.
 */
#define HANDLE_ERROR(err) (handle_error(err, __FILE__, __LINE__))