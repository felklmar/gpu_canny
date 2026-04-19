#pragma once

#include <vector>
#include <cstdint>

std::vector<uint8_t> detect_edges(const std::vector<uint8_t> & src_pixels, int img_width, int img_height, float sigma, float lower_threshold, float upper_threshold);
std::vector<float> create_gaussian_kernel(int k, float sigma);

__global__ void gaussian_blur(uint8_t* blurred_pixels, const uint8_t* src_pixels, int img_width, int img_height, const float* kernel, int k);
__global__ void compute_gradients(float* magnitudes, uint8_t* sectors, const uint8_t* blurred_pixels, int img_width, int img_height, const float* kernel_x, const float* kernel_y, int k);
__global__ void non_maximum_suppression(float* suppressed_magnitudes, const float* magnitudes, const uint8_t* sectors, int img_width, int img_height);
__global__ void double_thresholding(uint8_t* thresholded_pixels, const float* suppressed_magnitudes, size_t img_size, float lower_percent, float upper_percent);
__global__ void edge_hysteresis(uint8_t* edges, int img_width, int img_height, bool* d_changed);
__global__ void edge_hysteresis_cleanup(uint8_t* edges, size_t img_size);