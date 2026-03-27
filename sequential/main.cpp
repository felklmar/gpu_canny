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

    std::string inputName = argv[1];
    std::string outputName = argv[2];

    float sigma = std::stof(argv[3]);
    float lowerThreshold = std::stof(argv[4]);
    float upperThreshold = std::stof(argv[5]);

    auto [header, pixels] = loadImageFromFile(inputName);
    int w = header.Width, h = header.Height;

    // 1. Gaussian Blur
    std::vector<uint8_t> bluredPixels = measureTime("Gaussian Blur", [&]() {
        return gaussianBlur(pixels, w, h, sigma);
    });

    // 2. Gradients & Sectors
    auto [gradients, sectors] = measureTime("Compute Gradients", [&]() {
        return computeGradients(bluredPixels, w, h);
    });

    // 3. Non-Maximum Suppression
    std::vector<float> suppressedPixels = measureTime("Non-Max Suppression", [&]() {
        return nonMaximumSuppression(gradients, sectors, w, h);
    });

    // 4. Double Thresholding
    std::vector<uint8_t> thresholdedPixels = measureTime("Double Thresholding", [&]() {
        return doubleThresholding(suppressedPixels, lowerThreshold, upperThreshold);
    });

    // 5. Edge Hysteresis
    std::vector<uint8_t> finalEdges = measureTime("Edge Hysteresis", [&]() {
        return edgeHysteresis(thresholdedPixels, w, h);
    });

    saveImageToFile(outputName, header, finalEdges);

    return EXIT_SUCCESS;
}