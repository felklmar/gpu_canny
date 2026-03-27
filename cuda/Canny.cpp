#include "Canny.h"

#include <math.h>
#include <algorithm>
#include <iostream>

void create_gaussian_kernel(float* kernel, int k, float sigma) {
    int size = 2 * k + 1;
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

    for (int i = 0; i < size * size; ++i)
        kernel[i] /= sum;
}

void gaussian_blur(uint8_t* blurred_pixels, uint8_t* src_pixels, int img_width, int img_height, float sigma, int k) {
    int kernel_size = 2 * k + 1;
    float* kernel = new float[kernel_size * kernel_size];
    create_gaussian_kernel(kernel, k, sigma);

    for (size_t r = 0; r < static_cast<size_t>(img_height); ++r) {
        for (size_t c = 0; c < static_cast<size_t>(img_width); ++c) {
            float pixel_value = 0;
            for (int kr = 0; kr < kernel_size; ++kr) {
                for (int kc = 0; kc < kernel_size; ++kc) {
                    size_t nr = std::clamp(static_cast<int>(r) + (kr - k), 0, img_height - 1);
                    size_t nc = std::clamp(static_cast<int>(c) + (kc - k), 0, img_width - 1);
                    pixel_value += src_pixels[nr * img_width + nc] * kernel[kr * kernel_size + kc];
                }
            }

            blurred_pixels[r * img_width + c] = pixel_value;
        }
    }
    
    delete[] kernel;
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
                    size_t nr = std::clamp(static_cast<int>(r)+ (kr - k), 0, img_height - 1);
                    size_t nc = std::clamp(static_cast<int>(c) + (kc - k), 0, img_width - 1);
                    
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

    float lowerThreshold = max_mag * lower_percent;
    float upperThreshold = max_mag * upper_percent;

    for (size_t i = 0; i < img_size; ++i) {
        float mag = suppressed_magnitudes[i];
        if (mag >= upperThreshold)
            thresholded_edges[i] = 255;
        else if (mag >= lowerThreshold)
            thresholded_edges[i] = 128;
    }
}

void edge_hysteresis(uint8_t* pixels, int img_width, int img_height) {
    std::vector<int> edgesToProcess;
    for (int r = 1; r < img_height - 1; ++r) {
        for (int c = 1; c < img_width - 1; ++c) {
            int idx = r * img_width + c;
            if (pixels[idx] == 255)
                edgesToProcess.push_back(idx);
        }
    }

    while (!edgesToProcess.empty()) {
        size_t idx = edgesToProcess.back();
        edgesToProcess.pop_back();

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
                    edgesToProcess.push_back(nidx);
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