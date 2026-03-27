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
    size_t img_size = w * h;

    uint8_t* src_pixels = new uint8_t[img_size];
    uint8_t* blurred_pixels = new uint8_t[img_size];
    float*   magnitudes = new float[img_size];
    uint8_t* sectors = new uint8_t[img_size];
    float*   suppressed_edges = new float[img_size];

    for (size_t i = 0; i < img_size; i++) {
        src_pixels[i] = pixels[i];
    }

    // 1. Gaussian Blur
    gaussian_blur(blurred_pixels, src_pixels, w, h, sigma);

    // 2. Gradients & Sectors
    compute_gradients(magnitudes, sectors, blurred_pixels, w, h);

    // 3. Non-Maximum Suppression
    non_maximum_suppression(suppressed_edges, magnitudes, sectors, w, h);

    std::vector<float> suppressed;
    for (size_t i = 0; i < img_size; i++) {
        suppressed.push_back(suppressed_edges[i]);
    }

    // 4. Double Thresholding
    std::vector<uint8_t> thresholdedPixels = measureTime("Double Thresholding", [&]() {
        return doubleThresholding(suppressed, lowerThreshold, upperThreshold);
    });

    // 5. Edge Hysteresis
    std::vector<uint8_t> finalEdges = measureTime("Edge Hysteresis", [&]() {
        return edgeHysteresis(thresholdedPixels, w, h);
    });

    saveImageToFile(outputName, header, finalEdges);

    delete[] src_pixels;
    delete[] blurred_pixels;
    delete[] magnitudes;
    delete[] sectors;
    delete[] suppressed_edges;

    return EXIT_SUCCESS;
}