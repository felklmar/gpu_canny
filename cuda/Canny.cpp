#include "Canny.h"

#include <math.h>
#include <algorithm>
#include <iostream>

void create_gaussian_kernel(float* kernel, int k, float sigma) {
    int size = 2 * k + 1;
    float two_sigma_sq = 2.0f * sigma * sigma;
    
    float sum = 0.0f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float ny = y - k;
            float nx = x - k;
            
            float exponent = -(nx * nx + ny * ny) / two_sigma_sq;
            float value = std::exp(exponent);

            kernel[y * size + x] = value;
            sum += value;
        }
    }

    for (int i = 0; i < size * size; ++i)
        kernel[i] /= sum;
}

void gaussian_blur(uint8_t* dst_pixels, uint8_t* src_pixels, int img_width, int img_height, float sigma, int k) {
    int kernel_size = 2 * k + 1;
    float* kernel = new float[kernel_size * kernel_size];
    create_gaussian_kernel(kernel, k, sigma);

    for (size_t y = 0; y < static_cast<size_t>(img_height); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(img_width); ++x) {
            float pixel_value = 0;
            for (int ky = 0; ky < kernel_size; ++ky) {
                for (int kx = 0; kx < kernel_size; ++kx) {
                    size_t ny = std::clamp(static_cast<int>(y) + (ky - k), 0, img_height - 1);
                    size_t nx = std::clamp(static_cast<int>(x) + (kx - k), 0, img_width - 1);
                    pixel_value += src_pixels[ny * img_width + nx] * kernel[ky * kernel_size + kx];
                }
            }

            dst_pixels[y * img_width + x] = pixel_value;
        }
    }
    
    delete[] kernel;
}

void compute_gradients(float* magnitudes, uint8_t* sectors, uint8_t* src_pixels, int img_width, int img_height) {
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

    for (size_t y = 0; y < static_cast<size_t>(img_height); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(img_width); ++x) {
            float conv_X = 0, conv_Y = 0;
            for (int ky = 0; ky < kernel_size; ++ky) {
                for (int kx = 0; kx < kernel_size; ++kx) {
                    size_t ny = std::clamp(static_cast<int>(y)+ (ky - k), 0, img_height - 1);
                    size_t nx = std::clamp(static_cast<int>(x) + (kx - k), 0, img_width - 1);
                    
                    float pixel = src_pixels[ny * img_width + nx];
                    conv_X += pixel * kernel_X[ky * kernel_size + kx];
                    conv_Y += pixel * kernel_Y[ky * kernel_size + kx];
                }
            }

            magnitudes[y * img_width + x] = std::sqrt(conv_X * conv_X + conv_Y * conv_Y);
            
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

            sectors[y * img_width + x] = sector;
        }
    }
}

std::vector<float> nonMaximumSuppression(const std::vector<float> & gradients, const std::vector<uint8_t> & sectors, int w, int h) {
    std::vector<float> suppressedEdges(w * h, 0.0f);
    for (int i = 1; i < h - 1; ++i) {
        for (int j = 1; j < w - 1; ++j) {
            int idx = i * w + j;
            float grad = gradients[idx];
            
            if (grad < 1e-5f) continue;

            bool isLocalMax = true;
            switch (sectors[idx]) {
            case 0:
                isLocalMax = !(gradients[idx - 1] >= grad || gradients[idx + 1] > grad);          
                break;
            case 45:
                isLocalMax = !(gradients[idx - (w + 1)] >= grad || gradients[idx + (w + 1)] > grad);
                break;
            case 90:
                isLocalMax = !(gradients[idx - w] >= grad || gradients[idx + w] > grad);
                break;
            case 135:
                isLocalMax = !(gradients[idx - (w - 1)] >= grad || gradients[idx + (w - 1)] > grad);
                break;
            default:
                isLocalMax = false;
                break;
            }
            
            if (isLocalMax)
                suppressedEdges[idx] = grad;
        }
    }

    return suppressedEdges;
}

std::vector<uint8_t> doubleThresholding(const std::vector<float> & suppressed, float lowerPercent, float upperPercent) {
    size_t totalPixels = suppressed.size();
    std::vector<uint8_t> result(totalPixels, 0);

    auto maxIt = std::max_element(suppressed.begin(), suppressed.end());
    float maxGrad = (maxIt != suppressed.end()) ? *maxIt : 0.0f;

    if (maxGrad < 1e-5f) return result; 

    float lowerThreshold = maxGrad * lowerPercent;
    float upperThreshold = maxGrad * upperPercent;

    for (size_t i = 0; i < totalPixels; ++i) {
        float grad = suppressed[i];
        
        if (grad >= upperThreshold)
            result[i] = 255;
        else if (grad >= lowerThreshold)
            result[i] = 128;
    }

    return result;
}

std::vector<uint8_t> edgeHysteresis(const std::vector<uint8_t> & pixels, int w, int h) {
    std::vector<uint8_t> finalEdges = pixels;
    
    std::vector<int> edgesToProcess;
    for (int i = 1; i < h - 1; ++i) {
        for (int j = 1; j < w - 1; ++j) {
            int idx = i * w + j;
            if (finalEdges[idx] == 255)
                edgesToProcess.push_back(idx);
        }
    }

    while (!edgesToProcess.empty()) {
        int idx = edgesToProcess.back();
        edgesToProcess.pop_back();

        int r = idx / w;
        int c = idx % w;

        int neighbors[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };

        for (int i = 0; i < 8; ++i) {
            int nr = r + neighbors[i][0];
            int nc = c + neighbors[i][1];

            if (nr >= 0 && nr < h && nc >= 0 && nc < w) {
                int nIdx = nr * w + nc;
                if (finalEdges[nIdx] == 128) {
                    finalEdges[nIdx] = 255;
                    edgesToProcess.push_back(nIdx);
                }
            }
        }
    }

    int totalPixels = w * h;
    for (int i = 0; i < totalPixels; ++i) {
        if (finalEdges[i] == 128)
            finalEdges[i] = 0;
    }

    return finalEdges;
}