#include <iostream>
#include <chrono>

#include "ImageLoader.h"
#include "Canny.h"

template <typename Func>
decltype(auto) measureTime(const std::string & stepName, Func && func) {
    auto start = std::chrono::high_resolution_clock::now();

    decltype(auto) result = func(); 
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "[Timer] " << stepName << " took: " << duration.count() << " ms\n";
    return result;
}

int main(int argc, char const *argv[]) {
    if (argc < 5) {
        std::cout << "Usage: ./canny <input_picture> <output_name> <gamma> <lower_threshold> <upper_threshold>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_name = argv[1];
    std::string output_name = argv[2];

    float sigma = std::stof(argv[3]);
    float lower_threshold = std::stof(argv[4]);
    float upper_threshold = std::stof(argv[5]);

    auto [header, pixels] = loadImageFromFile(input_name);
    int w = header.Width, h = header.Height;
    size_t img_size = w * h;

    uint8_t* src_pixels = new uint8_t[img_size];
    uint8_t* blurred_pixels = new uint8_t[img_size];
    float*   magnitudes = new float[img_size];
    uint8_t* sectors = new uint8_t[img_size];
    float*   suppressed_magnitudes = new float[img_size];
    uint8_t* thresholded_pixels = new uint8_t[img_size];

    for (size_t i = 0; i < img_size; i++) {
        src_pixels[i] = pixels[i];
    }

    // 1. Gaussian Blur
    gaussian_blur(blurred_pixels, src_pixels, w, h, sigma);

    // 2. Gradients & Sectors
    compute_gradients(magnitudes, sectors, blurred_pixels, w, h);

    // 3. Non-Maximum Suppression
    non_maximum_suppression(suppressed_magnitudes, magnitudes, sectors, w, h);

    // 4. Double Thresholding
    double_thresholding(thresholded_pixels, suppressed_magnitudes, img_size, lower_threshold, upper_threshold);

    std::vector<uint8_t> thresholdedPixels;
    for (size_t i = 0; i < img_size; i++) {
        thresholdedPixels.push_back(thresholded_pixels[i]);
    }

    // 5. Edge Hysteresis
    std::vector<uint8_t> finalEdges = measureTime("Edge Hysteresis", [&]() {
        return edgeHysteresis(thresholdedPixels, w, h);
    });

    saveImageToFile(output_name, header, finalEdges);

    delete[] src_pixels;
    delete[] blurred_pixels;
    delete[] magnitudes;
    delete[] sectors;
    delete[] suppressed_magnitudes;
    delete[] thresholded_pixels;

    return EXIT_SUCCESS;
}