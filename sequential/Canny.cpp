/**
 * @file Canny.cpp
 * @brief Implementations for the sequential Canny Edge Detection pipeline.
 */

#include "Canny.h"

#include <math.h>
#include <algorithm>

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

std::vector<uint8_t> gaussian_blur(const std::vector<uint8_t> & src_pixels, int img_width, int img_height, float sigma, int k) {
    std::vector<float> kernel = create_gaussian_kernel(k, sigma);
    int kernel_size = 2 * k + 1;
    
    std::vector<uint8_t> blurred_pixels(img_width * img_height);
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

    return blurred_pixels;
}

std::pair<std::vector<float>, std::vector<uint8_t>> compute_gradients(const std::vector<uint8_t> & blurred_pixels, int img_width, int img_height) {
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

    std::vector<float> magnitudes(img_width * img_height);
    std::vector<uint8_t> sectors(img_width * img_height);
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
                sector = 1; // 45
            } else if (angle > 67.5 && angle <= 112.5) {
                sector = 2; // 90
            } else if (angle > 112.5 && angle <= 157.5) {
                sector = 3; // 135
            }

            sectors[r * img_width + c] = sector;
        }
    }

    return {magnitudes, sectors};
}

std::vector<float> non_maximum_suppression(const std::vector<float> & magnitudes, const std::vector<uint8_t> & sectors, int img_width, int img_height) {
    std::vector<float> suppressed_magnitudes(img_width * img_height, 0.0f);
    for (int r = 1; r < img_height - 1; ++r) {
        for (int c = 1; c < img_width - 1; ++c) {
            size_t idx = r * img_width + c;
            float mag = magnitudes[idx];
            
            if (mag < 1e-5f) continue;

            const int offsets[4] = {1, (img_width + 1), img_width, (img_width - 1)};
            int step = offsets[sectors[idx]];
            bool is_local_max = !(magnitudes[idx - step] >= mag || magnitudes[idx + step] > mag);
            suppressed_magnitudes[idx] = (is_local_max) ? mag : 0.0f;
        }
    }

    return suppressed_magnitudes;
}

std::vector<uint8_t> double_thresholding(const std::vector<float> & suppressed_magnitudes, float lower_percent, float upper_percent) {
    size_t img_size = suppressed_magnitudes.size();
    std::vector<uint8_t> thresholded_edges(img_size, 0);

    auto maxIt = std::max_element(suppressed_magnitudes.begin(), suppressed_magnitudes.end());
    float maxGrad = (maxIt != suppressed_magnitudes.end()) ? *maxIt : 0.0f;

    if (maxGrad < 1e-5f) return thresholded_edges; 

    float lower_threshold = maxGrad * lower_percent;
    float upper_threshold = maxGrad * upper_percent;

    for (size_t i = 0; i < img_size; ++i) {
        float mag = suppressed_magnitudes[i];
        if (mag >= upper_threshold)
            thresholded_edges[i] = 255;
        else if (mag >= lower_threshold)
            thresholded_edges[i] = 128;
    }

    return thresholded_edges;
}

std::vector<uint8_t> edge_hysteresis(const std::vector<uint8_t> & pixels, int img_width, int img_height) {
    std::vector<uint8_t> final_edges = pixels;
    
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
                if (final_edges[nidx] == 128) {
                    final_edges[nidx] = 255;
                    edges_to_process.push_back(nidx);
                }
            }
        }
    }

    size_t img_size = img_width * img_height;
    for (size_t i = 0; i < img_size; ++i) {
        if (final_edges[i] == 128)
            final_edges[i] = 0;
    }

    return final_edges;
}