/**
 * @file CudaTimer.cu
 * @brief Implementations for the CudaTimer class.
 */

#include "CudaTimer.h"

CudaTimer::CudaTimer() {
    HANDLE_ERROR(cudaEventCreate(&m_start));
    HANDLE_ERROR(cudaEventCreate(&m_stop));
}

CudaTimer::~CudaTimer() {
    HANDLE_ERROR(cudaEventDestroy(m_start));
    HANDLE_ERROR(cudaEventDestroy(m_stop));
}

void CudaTimer::start(cudaStream_t stream) {
    HANDLE_ERROR(cudaEventRecord(m_start, stream));
}

float CudaTimer::stop(cudaStream_t stream) {
    HANDLE_ERROR(cudaEventRecord(m_stop, stream));
    HANDLE_ERROR(cudaEventSynchronize(m_stop));

    float elapsed_time = 0.0f;
    HANDLE_ERROR(cudaEventElapsedTime(&elapsed_time, m_start, m_stop));
    return elapsed_time;
}