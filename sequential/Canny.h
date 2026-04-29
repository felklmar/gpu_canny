/**
 * @file Canny.h
 * @brief Declarations for the Canny Edge Detection pipeline functions.
 * * This file contains the sequential implementation steps for the Canny Edge 
 * Detection algorithm, including Gaussian blur, gradient computation, 
 * non-maximum suppression, double thresholding, and edge hysteresis.
 */

#pragma once

#include <vector>
#include <cstdint>
#include <utility>

/**
 * @brief Generates a 2D Gaussian kernel for image blurring.
 * * @param k The radius of the kernel. The total size will be (2k + 1) x (2k + 1).
 * @param sigma The standard deviation of the Gaussian distribution.
 * @return std::vector<float> A flattened 2D vector representing the normalized Gaussian kernel.
 */
std::vector<float> create_gaussian_kernel(int k, float sigma);

/**
 * @brief Applies a Gaussian blur to an image to reduce noise and detail.
 * * @param src_pixels A flattened vector of grayscale image pixels (0-255).
 * @param img_width The width of the image.
 * @param img_height The height of the image.
 * @param sigma The standard deviation for the Gaussian kernel (default: 1.0f).
 * @param k The radius of the Gaussian kernel (default: 2, yielding a 5x5 kernel).
 * @return std::vector<uint8_t> A new vector containing the blurred image pixels.
 */
std::vector<uint8_t> gaussian_blur(const std::vector<uint8_t> & src_pixels, int img_width, int img_height, float sigma = 1.0f, int k = 2);

/**
 * @brief Computes the gradient magnitudes and directions (sectors) using the Sobel operator.
 * * @param blurred_pixels The blurred grayscale image.
 * @param img_width The width of the image.
 * @param img_height The height of the image.
 * @return std::pair<std::vector<float>, std::vector<uint8_t>> A pair containing:
 * 1. A vector of gradient magnitudes.
 * 2. A vector of quantized gradient directions (sectors 0-3 corresponding to 0, 45, 90, 135 degrees).
 */
std::pair<std::vector<float>, std::vector<uint8_t>> compute_gradients(const std::vector<uint8_t> & blurred_pixels, int img_width, int img_height);

/**
 * @brief Thins edges by keeping only local maxima along the gradient direction.
 * * @param magnitudes The gradient magnitudes.
 * @param sectors The quantized gradient directions.
 * @param img_width The width of the image.
 * @param img_height The height of the image.
 * @return std::vector<float> A vector of suppressed gradient magnitudes (thin edges).
 */
std::vector<float> non_maximum_suppression(const std::vector<float> & magnitudes, const std::vector<uint8_t> & sectors, int img_width, int img_height);

/**
 * @brief Classifies edges into strong, weak, or non-edges based on magnitude thresholds.
 * * @param suppressed_magnitudes The thinned gradient magnitudes.
 * @param lower_percent The lower threshold as a percentage of the maximum gradient magnitude (0.0 to 1.0).
 * @param upper_percent The upper threshold as a percentage of the maximum gradient magnitude (0.0 to 1.0).
 * @return std::vector<uint8_t> A vector of edge classifications: 255 (strong), 128 (weak), or 0 (non-edge).
 */
std::vector<uint8_t> double_thresholding(const std::vector<float> & suppressed_magnitudes, float lower_percent, float upper_percent);

/**
 * @brief Finalizes edge detection by tracking weak edges connected to strong edges.
 * * @param pixels The thresholded image containing strong (255) and weak (128) edges.
 * @param img_width The width of the image.
 * @param img_height The height of the image.
 * @return std::vector<uint8_t> The final binary edge map where all edges are 255 and background is 0.
 */
std::vector<uint8_t> edge_hysteresis(const std::vector<uint8_t> & pixels, int img_width, int img_height);