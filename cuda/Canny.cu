/**
 * @file Canny.cu
 * @brief Implementations for the CUDA kernels and host functions for Edge Detection.
 */

#include "Canny.h"
#include "CudaUtils.h"
#include "CudaTimer.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#include <thrust/extrema.h>
#include <thrust/device_ptr.h>

std::vector<uint8_t> detect_edges(const std::vector<uint8_t> & h_src_pixels, 
                                  const std::pair<int, int> & img_dimensions, float sigma, 
                                  const std::pair<float, float> & thresholds,
                                  const std::pair<int, int> & block_dimensions_2D, int block_size_1D) {

    int img_width = img_dimensions.first;
    int img_height = img_dimensions.second;
    size_t img_size = img_width * img_height;
    
    cudaDeviceProp prop;
    int which_device;
    
    HANDLE_ERROR(cudaGetDevice(&which_device));
    HANDLE_ERROR(cudaGetDeviceProperties(&prop, which_device));

    std::cout << prop.name << "\n";

    uint8_t* d_src_pixels;
    uint8_t* d_blurred_pixels;
    float*   d_magnitudes;
    uint8_t* d_sectors;
    float*   d_suppressed_magnitudes;
    uint8_t* d_edges;

    HANDLE_ERROR(cudaMalloc((void**)&d_src_pixels, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMalloc((void**)&d_blurred_pixels, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMalloc((void**)&d_magnitudes, img_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&d_sectors, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMalloc((void**)&d_suppressed_magnitudes, img_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&d_edges, img_size * sizeof(uint8_t)));

    HANDLE_ERROR(cudaMemcpy(d_src_pixels, h_src_pixels.data(), img_size * sizeof(uint8_t), cudaMemcpyHostToDevice));

    const int gauss_size = 5 * 5;
    std::vector<float> gauss = create_gaussian_kernel(2, sigma);
    float* d_gauss;

    const float sobel_x[] = {
        -1.0f, 0.0f, 1.0f, 
        -2.0f, 0.0f, 2.0f, 
        -1.0f, 0.0f, 1.0f
    };

    const float sobel_y[] = {
        -1.0f, -2.0f, -1.0f,
         0.0f,  0.0f,  0.0f,
         1.0f,  2.0f,  1.0f
    };

    float* d_sobel_x;
    float* d_sobel_y;
    const int sobel_size = 3 * 3;

    HANDLE_ERROR(cudaMalloc((void**)&d_gauss, gauss_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&d_sobel_x, sobel_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&d_sobel_y, sobel_size * sizeof(float)));

    HANDLE_ERROR(cudaMemcpy(d_gauss, gauss.data(), gauss_size * sizeof(float), cudaMemcpyHostToDevice));
    HANDLE_ERROR(cudaMemcpy(d_sobel_x, sobel_x, sobel_size * sizeof(float), cudaMemcpyHostToDevice));
    HANDLE_ERROR(cudaMemcpy(d_sobel_y, sobel_y, sobel_size * sizeof(float), cudaMemcpyHostToDevice));

    dim3 threads_per_block_2D(block_dimensions_2D.first, block_dimensions_2D.second);
    dim3 blocks_per_grid_2D((img_width + threads_per_block_2D.x - 1) / threads_per_block_2D.x, (img_height + threads_per_block_2D.y - 1) / threads_per_block_2D.y);

    int threads_per_block_1D = block_size_1D;
    int blocks_per_grid_1D = (img_size + threads_per_block_1D - 1) / threads_per_block_1D;

    bool* d_changed;
    HANDLE_ERROR(cudaMallocManaged(&d_changed, sizeof(bool)));

    CudaTimer timer;
    float total_time = 0, t = 0;

    // 1. Gaussian Blur =============================================================================================================================================
    timer.start();
    gaussian_blur<<<blocks_per_grid_2D, threads_per_block_2D>>>(d_blurred_pixels, d_src_pixels, img_width, img_height, d_gauss, 2);
    t = timer.stop();
    total_time += t;
    std::cout << "[GPU] Gaussian Blur took: " << t << " ms\n";
    
    // 2. Gradients & Sectors =======================================================================================================================================
    timer.start();
    compute_gradients<<<blocks_per_grid_2D, threads_per_block_2D>>>(d_magnitudes, d_sectors, d_blurred_pixels, img_width, img_height, d_sobel_x, d_sobel_y, 1);
    t = timer.stop();
    total_time += t;
    std::cout << "[GPU] Gradients took: " << t << " ms\n";
    
    // 3. Non-Maximum Suppression ===================================================================================================================================
    non_maximum_suppression<<<blocks_per_grid_2D, threads_per_block_2D>>>(d_suppressed_magnitudes, d_magnitudes, d_sectors, img_width, img_height);
    t = timer.stop();
    total_time += t;
    std::cout << "[GPU] Non-Maximum Suppression took: " << t << " ms\n";

    // 4. Double Thresholding =======================================================================================================================================
    timer.start();

    thrust::device_ptr<float> dev_ptr = thrust::device_pointer_cast(d_suppressed_magnitudes);
    float max_mag = *(thrust::max_element(dev_ptr, dev_ptr + img_size));

    float lower_threshold = max_mag * thresholds.first;
    float upper_threshold = max_mag * thresholds.second;

    double_thresholding<<<blocks_per_grid_1D, threads_per_block_1D>>>(d_edges, d_suppressed_magnitudes, img_size, lower_threshold, upper_threshold);
    t = timer.stop();
    total_time += t;
    std::cout << "[GPU] Double Thresholding took: " << t << " ms\n";

    // 5. Edge Hysteresis ===========================================================================================================================================
    timer.start();

    size_t iterations = 0;
    *d_changed = true;

    while (*d_changed) {
        *d_changed = false;
        edge_hysteresis<<<blocks_per_grid_2D, threads_per_block_2D>>>(d_edges, img_width, img_height, d_changed);
        HANDLE_ERROR(cudaDeviceSynchronize()); 
        iterations++;
    }

    edge_hysteresis_cleanup<<<blocks_per_grid_1D, threads_per_block_1D>>>(d_edges, img_size);
    HANDLE_ERROR(cudaDeviceSynchronize());
    t = timer.stop();
    total_time += t;
    std::cout << "[GPU] Edge Hysteresis took: " << t << " ms (" << iterations << " iterations)\n";
    
    std::cout << "Total time taken: " << total_time << " ms" << std::endl;

    std::vector<uint8_t> h_edges(img_size);
    HANDLE_ERROR(cudaMemcpy(h_edges.data(), d_edges, img_size * sizeof(uint8_t), cudaMemcpyDeviceToHost));

    HANDLE_ERROR(cudaFree(d_src_pixels));
    HANDLE_ERROR(cudaFree(d_blurred_pixels));
    HANDLE_ERROR(cudaFree(d_magnitudes));
    HANDLE_ERROR(cudaFree(d_sectors));
    HANDLE_ERROR(cudaFree(d_suppressed_magnitudes));
    HANDLE_ERROR(cudaFree(d_edges));
    
    HANDLE_ERROR(cudaFree(d_gauss));
    HANDLE_ERROR(cudaFree(d_sobel_x));
    HANDLE_ERROR(cudaFree(d_sobel_y));

    HANDLE_ERROR(cudaFree(d_changed));

    return h_edges;
}

std::vector<float> create_gaussian_kernel(int k, float sigma) {
    int size = 2 * k + 1;
    std::vector<float> kernel(size * size);
    float two_sigma_sq = 2.0f * sigma * sigma;
    
    float sum = 0.0f;
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            float nr = r - k;
            float nc = c - k;
            
            float exponent = -(nc * nc + nr * nr) / two_sigma_sq;
            float value = std::exp(exponent);

            kernel[r * size + c] = value;
            sum += value;
        }
    }

    for (float & weight : kernel)
        weight /= sum;

    return kernel;
}

__global__ void gaussian_blur(uint8_t* blurred_pixels, const uint8_t* src_pixels, int img_width, int img_height, const float* kernel, int k) {
    int kernel_size = 2 * k + 1;
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    int c = blockIdx.x * blockDim.x + threadIdx.x;

    if (r < img_height && c < img_width) {
        float pixel_value = 0.0f;
        for (int kr = 0; kr < kernel_size; ++kr) {
            for (int kc = 0; kc < kernel_size; ++kc) {
                int nr = max(0, min(r + (kr - k), img_height - 1)); 
                int nc = max(0, min(c + (kc - k), img_width - 1)); 
                
                pixel_value += src_pixels[nr * img_width + nc] * kernel[kr * kernel_size + kc];
            }
        }

        blurred_pixels[r * img_width + c] = static_cast<uint8_t>(min(max(pixel_value, 0.0f), 255.0f));
    }
}

__global__ void compute_gradients(float* magnitudes, uint8_t* sectors, const uint8_t* blurred_pixels, int img_width, int img_height, const float* kernel_x, const float* kernel_y, int k) {   
    int kernel_size = 2 * k + 1;
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    int c = blockIdx.x * blockDim.x + threadIdx.x;

    if (r < img_height && c < img_width) {
        float conv_x = 0, conv_y = 0;
        for (int kr = 0; kr < kernel_size; ++kr) {
            for (int kc = 0; kc < kernel_size; ++kc) {
                size_t nr = max(0, min(static_cast<int>(r) + (kr - k), img_height - 1)); 
                size_t nc = max(0, min(static_cast<int>(c) + (kc - k), img_width - 1)); 

                float pixel = blurred_pixels[nr * img_width + nc];
                conv_x += pixel * kernel_x[kr * kernel_size + kc];
                conv_y += pixel * kernel_y[kr * kernel_size + kc];
            }
        }

        magnitudes[r * img_width + c] = std::sqrt(conv_x * conv_x + conv_y * conv_y);
        
        float angle = std::atan2(conv_y, conv_x) * 180.0f / M_PI;
        if (angle < 0) angle += 180.0f;

        uint8_t sector = 0;
        if (angle > 22.5 && angle <= 67.5) {
            sector = 1; // 45
        } else if (angle > 67.5 && angle <= 112.5) {
            sector = 2; // 90
        } else if (angle > 112.5 && angle <= 157.5) {
            sector = 3; // 135
        }

        sectors[r * img_width + c] = sector;
    }
}

__global__ void non_maximum_suppression(float* suppressed_magnitudes, const float* magnitudes, const uint8_t* sectors, int img_width, int img_height) {
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    int c = blockIdx.x * blockDim.x + threadIdx.x;

    if (r < img_height && c < img_width) {
        size_t idx = r * img_width + c;
        
        if (r == 0 || c == 0 || r == img_height - 1 || c == img_width - 1) {
            suppressed_magnitudes[idx] = 0.0f;
            return;
        }
        
        float mag = magnitudes[idx];
        
        const int offsets[4] = {1, (img_width + 1), img_width, (img_width - 1)};
        int step = offsets[sectors[idx]];
        bool is_local_max = !(magnitudes[idx - step] >= mag || magnitudes[idx + step] > mag);
        suppressed_magnitudes[idx] = (is_local_max) ? mag : 0.0f;
    }
}

__global__ void double_thresholding(uint8_t* thresholded_edges, const float* suppressed_magnitudes, size_t img_size, float lower_threshold, float upper_threshold) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < img_size) {
        float mag = suppressed_magnitudes[idx];
        if (mag >= upper_threshold)
            thresholded_edges[idx] = 255;
        else if (mag >= lower_threshold)
            thresholded_edges[idx] = 128;
        else
            thresholded_edges[idx] = 0;
    }
}

__global__ void edge_hysteresis(uint8_t* edges, int img_width, int img_height, bool* d_changed) {
int c = blockIdx.x * blockDim.x + threadIdx.x;
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    int idx = r * img_width + c;

    bool is_valid_pixel = (r > 0 && r < img_height - 1 && c > 0 && c < img_width - 1);
    
    __shared__ bool s_changed;

    do {
        if (threadIdx.x == 0 && threadIdx.y == 0)
            s_changed = false;
        __syncthreads(); 

        bool promoted_this_round = false;

        if (is_valid_pixel && edges[idx] == 128) {
            if (edges[idx - img_width - 1] == 255 || edges[idx - img_width] == 255 || edges[idx - img_width + 1] == 255 ||
                edges[idx - 1] == 255             ||                                  edges[idx + 1] == 255 ||
                edges[idx + img_width - 1] == 255 || edges[idx + img_width] == 255 || edges[idx + img_width + 1] == 255) {
                
                edges[idx] = 255;
                s_changed = promoted_this_round = true;
            }
        }
        __syncthreads(); 

        if (promoted_this_round && (threadIdx.x == 0 || threadIdx.x == blockDim.x - 1 || threadIdx.y == 0 || threadIdx.y == blockDim.y - 1))
                *d_changed = true;

    } while (s_changed);
}

__global__ void edge_hysteresis_cleanup(uint8_t* edges, size_t img_size) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < img_size && edges[idx] == 128)
        edges[idx] = 0;
}