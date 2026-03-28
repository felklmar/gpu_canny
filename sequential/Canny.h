#pragma once

#include <vector>
#include <cstdint>

std::vector<float> create_gaussian_kernel(int k, float sigma);
std::vector<uint8_t> gaussian_blur(const std::vector<uint8_t> & src_pixels, int img_width, int img_height, float sigma = 1.0f, int k = 2);
std::pair<std::vector<float>, std::vector<uint8_t>> compute_gradients(const std::vector<uint8_t> & blurred_pixels, int img_width, int img_height);
std::vector<float> non_maximum_suppression(const std::vector<float> & magnitudes, const std::vector<uint8_t> & sectors, int img_width, int img_height);
std::vector<uint8_t> double_thresholding(const std::vector<float> & suppressed_magnitudes, float lower_percent, float upper_percent);
std::vector<uint8_t> edge_hysteresis(const std::vector<uint8_t> & pixels, int img_width, int img_height);