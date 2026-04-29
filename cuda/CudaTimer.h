/**
 * @file CudaTimer.h
 * @brief A lightweight, RAII-style wrapper for CUDA events to measure GPU execution time.
 */

#pragma once

#include "CudaUtils.h"

/**
 * @class CudaTimer
 * @brief Measures the elapsed time of GPU operations using cudaEvent_t.
 */
class CudaTimer {
    private:
        cudaEvent_t m_start, m_stop;
    public:
        /**
         * @brief Constructs the timer and allocates internal CUDA events.
         */
        CudaTimer();

        /**
         * @brief Destroys the timer and frees internal CUDA events.
         */
        ~CudaTimer();

        /**
         * @brief Records the start event in the specified CUDA stream.
         * @param stream The CUDA stream to record the event in (default: 0).
         */
        void start(cudaStream_t stream = 0);

        /**
         * @brief Records the stop event, synchronizes the device, and calculates elapsed time.
         * @param stream The CUDA stream to record the event in (default: 0).
         * @return float The elapsed time between start() and stop() in milliseconds.
         */
        float stop(cudaStream_t stream = 0);
};