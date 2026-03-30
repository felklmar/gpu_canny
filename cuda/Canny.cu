#include "Canny.h"

#include <math.h>
#include <iostream>
#include <algorithm>

// zpracování chyb
// error handling
static void handle_error(cudaError_t err, const char* file, int line) {
    if (err != cudaSuccess) {
        printf("%s in %s at line %d\n", cudaGetErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}

#define HANDLE_ERROR(err) (handle_error(err, __FILE__, __LINE__))

std::vector<uint8_t> detect_edges(std::vector<uint8_t> src_pixels, int img_width, int img_height, float sigma, float lower_threshold, float upper_threshold) {
    size_t img_size = img_width * img_height;

    uint8_t* original_pixels;
    uint8_t* blurred_pixels;
    float*   magnitudes = new float[img_size];
    uint8_t* sectors = new uint8_t[img_size];
    float*   suppressed_magnitudes = new float[img_size];
    uint8_t* edges = new uint8_t[img_size];

    int kernel_size = 5;
    std::vector<float> gauss_kernel = create_gaussian_kernel(2, sigma);
    float* gauss_kernel_gpu;

    cudaDeviceProp prop;
    int which_device;

    HANDLE_ERROR(cudaGetDevice(&which_device));
    HANDLE_ERROR(cudaGetDeviceProperties(&prop, which_device));

    HANDLE_ERROR(cudaMallocManaged((void**)&original_pixels, img_size * sizeof(uint8_t)));
    std::copy(src_pixels.begin(), src_pixels.end(), original_pixels);

    HANDLE_ERROR(cudaMallocManaged((void**)&blurred_pixels, img_size * sizeof(uint8_t)));

    HANDLE_ERROR(cudaMalloc((void**)&gauss_kernel_gpu, kernel_size * kernel_size * sizeof(float)));
    HANDLE_ERROR(cudaMemcpy(gauss_kernel_gpu, gauss_kernel.data(), kernel_size * kernel_size * sizeof(float), cudaMemcpyHostToDevice));

    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((img_width + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                   (img_height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    cudaEvent_t start, stop;
    float elapsed_time;

    HANDLE_ERROR(cudaEventCreate(&start));
    HANDLE_ERROR(cudaEventCreate(&stop));
    HANDLE_ERROR(cudaEventRecord(start, 0));

    // 1. Gaussian Blur
    gaussian_blur<<<numBlocks, threadsPerBlock>>>(blurred_pixels, original_pixels, img_width, img_height, gauss_kernel_gpu, 2);

    HANDLE_ERROR(cudaDeviceSynchronize());
    HANDLE_ERROR(cudaEventRecord(stop, 0));
    HANDLE_ERROR(cudaEventSynchronize(stop));
	HANDLE_ERROR(cudaEventElapsedTime(&elapsed_time, start, stop));

    std::cout << "[GPU] Gaussian Blur took: " << elapsed_time << " ms\n";

    // 2. Gradients & Sectors
    compute_gradients(magnitudes, sectors, blurred_pixels, img_width, img_height);
    std::cout << "Gradients done" << std::endl;

    // 3. Non-Maximum Suppression
    non_maximum_suppression(suppressed_magnitudes, magnitudes, sectors, img_width, img_height);
    std::cout << "Non-Maximum Suppression done" << std::endl;

    // 4. Double Thresholding
    double_thresholding(edges, suppressed_magnitudes, img_size, lower_threshold, upper_threshold);
    std::cout << "Double Thresholding done" << std::endl;

    // 5. Edge Hysteresis
    edge_hysteresis(edges, img_width, img_height);
    std::cout << "Hysteresis done" << std::endl;

    std::vector<uint8_t> detected_edges(edges, edges + img_size * sizeof(uint8_t));

    HANDLE_ERROR(cudaFree(original_pixels));
    HANDLE_ERROR(cudaFree(blurred_pixels));
    HANDLE_ERROR(cudaFree(gauss_kernel_gpu));
    
    HANDLE_ERROR(cudaEventDestroy(start));
	HANDLE_ERROR(cudaEventDestroy(stop));

    delete[] magnitudes;
    delete[] sectors;
    delete[] suppressed_magnitudes;
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

__global__ void gaussian_blur(uint8_t* blurred_pixels, uint8_t* src_pixels, int img_width, int img_height, const float* kernel, int k) {
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

void compute_gradients(float* magnitudes, uint8_t* sectors, uint8_t* blurred_pixels, int img_width, int img_height) {
    float kernel_X[] = {
        -1.0f, 0.0f, 1.0f, 
        -2.0f, 0.0f, 2.0f, 
        -1.0f, 0.0f, 1.0f
    };

    float kernel_Y[] = {
        -1.0f, -2.0f, -1.0f,
         0.0f,  0.0f,  0.0f,
         1.0f,  2.0f,  1.0f
    };
    
    const int k = 1;
    const int kernel_size = 2 * k + 1;

    for (size_t r = 0; r < static_cast<size_t>(img_height); ++r) {
        for (size_t c = 0; c < static_cast<size_t>(img_width); ++c) {
            float conv_X = 0, conv_Y = 0;
            for (int kr = 0; kr < kernel_size; ++kr) {
                for (int kc = 0; kc < kernel_size; ++kc) {
                    size_t nr = max(0, min(static_cast<int>(r) + (kr - k), img_height - 1)); 
                    size_t nc = max(0, min(static_cast<int>(c) + (kc - k), img_width - 1)); 

                    float pixel = blurred_pixels[nr * img_width + nc];
                    conv_X += pixel * kernel_X[kr * kernel_size + kc];
                    conv_Y += pixel * kernel_Y[kr * kernel_size + kc];
                }
            }

            magnitudes[r * img_width + c] = std::sqrt(conv_X * conv_X + conv_Y * conv_Y);
            
            float angle = std::atan2(conv_Y, conv_X) * 180.0f / M_PI;
            if (angle < 0) angle += 180.0f;

            uint8_t sector = 0;
            if (angle > 22.5 && angle <= 67.5) {
                sector = 45;
            } else if (angle > 67.5 && angle <= 112.5) {
                sector = 90;
            } else if (angle > 112.5 && angle <= 157.5) {
                sector = 135; 
            }

            sectors[r * img_width + c] = sector;
        }
    }
}

void non_maximum_suppression(float* suppressed_magnitudes, float* magnitudes, uint8_t* sectors, int img_width, int img_height) {
    for (int r = 1; r < img_height - 1; ++r) {
        for (int c = 1; c < img_width - 1; ++c) {
            size_t idx = r * img_width + c;
            float mag = magnitudes[idx];
            
            if (mag < 1e-5f) continue;

            bool is_local_max = true;
            switch (sectors[idx]) {
            case 0:
                is_local_max = !(magnitudes[idx - 1] >= mag || magnitudes[idx + 1] > mag);          
                break;
            case 45:
                is_local_max = !(magnitudes[idx - (img_width + 1)] >= mag || magnitudes[idx + (img_width + 1)] > mag);
                break;
            case 90:
                is_local_max = !(magnitudes[idx - img_width] >= mag || magnitudes[idx + img_width] > mag);
                break;
            case 135:
                is_local_max = !(magnitudes[idx - (img_width - 1)] >= mag || magnitudes[idx + (img_width - 1)] > mag);
                break;
            default:
                is_local_max = false;
                break;
            }
            
            if (is_local_max)
                suppressed_magnitudes[idx] =  mag;
        }
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