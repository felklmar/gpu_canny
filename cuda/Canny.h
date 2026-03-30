#pragma once

#include <vector>
#include <cstdint>

void create_gaussian_kernel(float* kernel, int k, float sigma);
__global__ void gaussian_blur(uint8_t* blurred_pixels, uint8_t* src_pixels, int img_width, int img_height, const float* kernel, int k);
void compute_gradients(float* magnitudes, uint8_t* sectors, uint8_t* blurred_pixels, int img_width, int img_height);
void non_maximum_suppression(float* suppressed_magnitudes, float* magnitudes, uint8_t* sectors, int img_width, int img_height);
void double_thresholding(uint8_t* thresholded_pixels, float* suppressed_magnitudes, size_t img_size, float lower_percent, float upper_percent);
void edge_hysteresis(uint8_t* pixels, int img_width, int img_height);