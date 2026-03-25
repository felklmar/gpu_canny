#pragma once

#include <vector>
#include <cstdint>

std::vector<float> createGaussianKernel(int k, float sigma);
std::vector<uint8_t> applyGauss(const std::vector<uint8_t> & pixels, int w, int h, float sigma, int k = 2);
std::pair<std::vector<float>, std::vector<uint8_t>> computeGradients(const std::vector<uint8_t> & pixels, int w, int h);
std::vector<float> nonMaximumSuppression(const std::vector<float> & gradients, const std::vector<uint8_t> & sectors, int w, int h);
std::vector<uint8_t> doubleThresholding(const std::vector<float> & suppressed, int w, int h, int lowerThreshold, int upperThreshold);
std::vector<uint8_t> applyHysteresis(const std::vector<uint8_t> & pixels, int w, int h);