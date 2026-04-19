#pragma once

#include "CudaUtils.h"

class CudaTimer {
    private:
        cudaEvent_t m_start, m_stop;
    public:
        CudaTimer();
        ~CudaTimer();
        void start(cudaStream_t stream = 0);
        float stop(cudaStream_t stream = 0);
};