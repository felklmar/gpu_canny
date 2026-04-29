/**
 * @file Canny.h
 * @brief Declarations for the CUDA-accelerated Canny Edge Detection pipeline.
 * * This file contains the host wrapper function for executing the pipeline, 
 * as well as the declarations for the individual CUDA kernels that perform 
 * blurring, gradient computation, suppression, thresholding, and hysteresis.
 */

#pragma once

#include <vector>
#include <cstdint>

/**
 * @brief Orchestrates the entire Canny edge detection pipeline on the GPU.
 * * @param src_pixels The flattened grayscale input image.
 * @param img_dimensions A pair containing {width, height} of the image.
 * @param sigma The standard deviation for the Gaussian blur.
 * @param thresholds A pair containing {lower_percent, upper_percent} for double thresholding.
 * @param block_dimensions_2D A pair containing {width, height} for the 2D CUDA thread blocks.
 * @param block_size_1D The number of threads per block for 1D CUDA kernels.
 * @return std::vector<uint8_t> The final binary edge map downloaded from the GPU.
 */
std::vector<uint8_t> detect_edges(const std::vector<uint8_t> & src_pixels,
                                  const std::pair<int, int> & img_dimensions, float sigma,
                                  const std::pair<float, float> & thresholds,
                                  const std::pair<int, int> & block_dimensions_2D, int block_size_1D);

/**
 * @brief Generates a 1D representation of a 2D Gaussian kernel on the host.
 * * @param k The radius of the kernel (total size = 2k + 1).
 * @param sigma The standard deviation of the Gaussian distribution.
 * @return std::vector<float> A flattened 2D vector representing the normalized Gaussian kernel.
 */
std::vector<float> create_gaussian_kernel(int k, float sigma);

/**
 * @brief CUDA kernel to apply a Gaussian blur to the image.
 * * @param blurred_pixels Output array for the blurred image.
 * @param src_pixels Input array of the original grayscale image.
 * @param img_width Width of the image.
 * @param img_height Height of the image.
 * @param kernel The flattened Gaussian kernel allocated on device memory.
 * @param k The radius of the Gaussian kernel.
 */
__global__ void gaussian_blur(uint8_t* blurred_pixels, const uint8_t* src_pixels, int img_width, int img_height, const float* kernel, int k);

/**
 * @brief CUDA kernel to compute gradient magnitudes and directions (sectors).
 * * @param magnitudes Output array for gradient magnitudes.
 * @param sectors Output array for quantized gradient directions (0-3).
 * @param blurred_pixels Input array of the blurred image.
 * @param img_width Width of the image.
 * @param img_height Height of the image.
 * @param kernel_x The Sobel operator kernel for the X-axis.
 * @param kernel_y The Sobel operator kernel for the Y-axis.
 * @param k The radius of the Sobel kernels (typically 1 for a 3x3 kernel).
 */
__global__ void compute_gradients(float* magnitudes, uint8_t* sectors, const uint8_t* blurred_pixels, int img_width, int img_height, const float* kernel_x, const float* kernel_y, int k);

/**
 * @brief CUDA kernel to thin edges via non-maximum suppression.
 * * @param suppressed_magnitudes Output array for thinned gradient magnitudes.
 * @param magnitudes Input array of the computed gradient magnitudes.
 * @param sectors Input array of the quantized gradient directions.
 * @param img_width Width of the image.
 * @param img_height Height of the image.
 */
__global__ void non_maximum_suppression(float* suppressed_magnitudes, const float* magnitudes, const uint8_t* sectors, int img_width, int img_height);

/**
 * @brief CUDA kernel to classify pixels into strong, weak, or non-edges.
 * * @param thresholded_pixels Output array containing 255 (strong), 128 (weak), or 0 (non-edge).
 * @param suppressed_magnitudes Input array of thinned gradient magnitudes.
 * @param img_size Total number of pixels in the image.
 * @param lower_threshold The absolute lower magnitude threshold.
 * @param upper_threshold The absolute upper magnitude threshold.
 */
__global__ void double_thresholding(uint8_t* thresholded_pixels, const float* suppressed_magnitudes, size_t img_size, float lower_threshold, float upper_threshold);

/**
 * @brief CUDA kernel to iteratively track and promote weak edges connected to strong edges.
 * * @param edges In/Out array of the thresholded edges. Updated in-place.
 * @param img_width Width of the image.
 * @param img_height Height of the image.
 * @param d_changed Pointer to a globally managed boolean flag indicating if any changes occurred.
 */
__global__ void edge_hysteresis(uint8_t* edges, int img_width, int img_height, bool* d_changed);

/**
 * @brief CUDA kernel to clean up remaining unpromoted weak edges.
 * * @param edges In/Out array of the edges. Any remaining 128s are set to 0.
 * @param img_size Total number of pixels in the image.
 */
__global__ void edge_hysteresis_cleanup(uint8_t* edges, size_t img_size);