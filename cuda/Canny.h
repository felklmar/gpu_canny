#pragma once

#include <vector>
#include <cstdint>

void create_gaussian_kernel(float* kernel, int k, float sigma);
void gaussian_blur(uint8_t* dst_pixels, uint8_t* src_pixels, int img_width, int img_height, float sigma = 1.0f, int k = 2);
void compute_gradients(float* magnitudes, uint8_t* sectors, uint8_t* src_pixels, int img_width, int mg_height);
std::vector<float> nonMaximumSuppression(const std::vector<float> & gradients, const std::vector<uint8_t> & sectors, int w, int h);
std::vector<uint8_t> doubleThresholding(const std::vector<float> & suppressed, float lowerPercent, float upperPercent);
std::vector<uint8_t> edgeHysteresis(const std::vector<uint8_t> & pixels, int w, int h);