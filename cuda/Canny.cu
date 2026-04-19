#include "Canny.h"
#include "CudaUtils.h"
#include "CudaTimer.h"

#include <math.h>
#include <iostream>
#include <algorithm>

std::vector<uint8_t> detect_edges(const std::vector<uint8_t> & src_pixels, int img_width, int img_height, float sigma, float lower_threshold, float upper_threshold) {
    size_t img_size = img_width * img_height;
    cudaDeviceProp prop;
    int which_device;
    
    HANDLE_ERROR(cudaGetDevice(&which_device));
    HANDLE_ERROR(cudaGetDeviceProperties(&prop, which_device));

    uint8_t* GPU_src_pixels;
    uint8_t* GPU_blurred_pixels;
    float*   GPU_magnitudes;
    uint8_t* GPU_sectors;
    float*   GPU_suppressed_magnitudes;
    uint8_t* edges = new uint8_t[img_size];

    HANDLE_ERROR(cudaMalloc((void**)&GPU_src_pixels, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMalloc((void**)&GPU_blurred_pixels, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMalloc((void**)&GPU_magnitudes, img_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&GPU_sectors, img_size * sizeof(uint8_t)));
    HANDLE_ERROR(cudaMallocManaged((void**)&GPU_suppressed_magnitudes, img_size * sizeof(float)));

    HANDLE_ERROR(cudaMemcpy(GPU_src_pixels, src_pixels.data(), img_size * sizeof(uint8_t), cudaMemcpyHostToDevice));

    const int gauss_size = 5 * 5;
    std::vector<float> gauss = create_gaussian_kernel(2, sigma);
    float* GPU_gauss;

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

    float* GPU_sobel_x;
    float* GPU_sobel_y;
    const int sobel_size = 3 * 3;

    HANDLE_ERROR(cudaMalloc((void**)&GPU_gauss, gauss_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&GPU_sobel_x, sobel_size * sizeof(float)));
    HANDLE_ERROR(cudaMalloc((void**)&GPU_sobel_y, sobel_size * sizeof(float)));

    HANDLE_ERROR(cudaMemcpy(GPU_gauss, gauss.data(), gauss_size * sizeof(float), cudaMemcpyHostToDevice));
    HANDLE_ERROR(cudaMemcpy(GPU_sobel_x, sobel_x, sobel_size * sizeof(float), cudaMemcpyHostToDevice));
    HANDLE_ERROR(cudaMemcpy(GPU_sobel_y, sobel_y, sobel_size * sizeof(float), cudaMemcpyHostToDevice));

    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((img_width + threadsPerBlock.x - 1) / threadsPerBlock.x, (img_height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    CudaTimer timer;

    // 1. Gaussian Blur
    timer.start();
    gaussian_blur<<<numBlocks, threadsPerBlock>>>(GPU_blurred_pixels, GPU_src_pixels, img_width, img_height, GPU_gauss, 2);
    std::cout << "[GPU] Gaussian Blur took: " << timer.stop() << " ms\n";

    // 2. Gradients & Sectors
    timer.start();
    compute_gradients<<<numBlocks, threadsPerBlock>>>(GPU_magnitudes, GPU_sectors, GPU_blurred_pixels, img_width, img_height, GPU_sobel_x, GPU_sobel_y, 1);
    std::cout << "[GPU] Gradients took: " << timer.stop() << " ms\n";
    
    // 3. Non-Maximum Suppression
    non_maximum_suppression<<<numBlocks, threadsPerBlock>>>(GPU_suppressed_magnitudes, GPU_magnitudes, GPU_sectors, img_width, img_height);
    std::cout << "[GPU] Non-Maximum Suppression took: " << timer.stop() << " ms\n";

    // 4. Double Thresholding
    timer.start();
    double_thresholding(edges, GPU_suppressed_magnitudes, img_size, lower_threshold, upper_threshold);
    std::cout << "[CPU] Double Thresholding took: " << timer.stop() << " ms\n";

    // 5. Edge Hysteresis
    timer.start();
    edge_hysteresis(edges, img_width, img_height);
    std::cout << "[CPU] Hysteresis took: " << timer.stop() << " ms\n";

    std::vector<uint8_t> detected_edges(edges, edges + img_size * sizeof(uint8_t));

    HANDLE_ERROR(cudaFree(GPU_src_pixels));
    HANDLE_ERROR(cudaFree(GPU_blurred_pixels));
    HANDLE_ERROR(cudaFree(GPU_magnitudes));
    HANDLE_ERROR(cudaFree(GPU_sectors));
    HANDLE_ERROR(cudaFree(GPU_suppressed_magnitudes));
    
    HANDLE_ERROR(cudaFree(GPU_gauss));
    HANDLE_ERROR(cudaFree(GPU_sobel_x));
    HANDLE_ERROR(cudaFree(GPU_sobel_y));
    
    delete[] edges;

    return detected_edges;
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

void double_thresholding(uint8_t* thresholded_edges, float* suppressed_magnitudes, size_t img_size,  float lower_percent, float upper_percent) {
    if (img_size == 0) return;

    float* max_ptr = std::max_element(suppressed_magnitudes, suppressed_magnitudes + img_size);
    float max_mag = (max_ptr != nullptr) ? *max_ptr : 0.0f;

    if (max_mag < 1e-5f) {
        std::fill(thresholded_edges, thresholded_edges + img_size, 0);
        return;
    }

    float lower_threshold = max_mag * lower_percent;
    float upper_threshold = max_mag * upper_percent;

    for (size_t i = 0; i < img_size; ++i) {
        float mag = suppressed_magnitudes[i];
        if (mag >= upper_threshold)
            thresholded_edges[i] = 255;
        else if (mag >= lower_threshold)
            thresholded_edges[i] = 128;
    }
}

void edge_hysteresis(uint8_t* pixels, int img_width, int img_height) {
    std::vector<int> edges_to_process;
    for (int r = 1; r < img_height - 1; ++r) {
        for (int c = 1; c < img_width - 1; ++c) {
            int idx = r * img_width + c;
            if (pixels[idx] == 255)
                edges_to_process.push_back(idx);
        }
    }

    while (!edges_to_process.empty()) {
        size_t idx = edges_to_process.back();
        edges_to_process.pop_back();

        int r = idx / img_width;
        int c = idx % img_width;

        int neighbors[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };

        for (int i = 0; i < 8; ++i) {
            int nr = r + neighbors[i][0];
            int nc = c + neighbors[i][1];

            if (nr >= 0 && nr < img_height && nc >= 0 && nc < img_width) {
                size_t nidx = nr * img_width + nc;
                if (pixels[nidx] == 128) {
                    pixels[nidx] = 255;
                    edges_to_process.push_back(nidx);
                }
            }
        }
    }

    size_t img_size = img_width * img_height;
    for (size_t i = 0; i < img_size; ++i) {
        if (pixels[i] == 128)
            pixels[i] = 0;
    }
}