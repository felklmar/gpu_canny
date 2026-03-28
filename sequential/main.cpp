#include <iostream>
#include <chrono>

#include "ImageLoader.h"
#include "Canny.h"

template <typename Func>
decltype(auto) measure_time(const std::string & step_name, Func && func) {
    auto start = std::chrono::high_resolution_clock::now();

    decltype(auto) result = func(); 
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "[Timer] " << step_name << " took: " << duration.count() << " ms\n";
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

    auto [header, pixels] = load_image_from_file(input_name);
    int w = header.Width, h = header.Height;

    // 1. Gaussian Blur
    std::vector<uint8_t> blurred_pixels = measure_time("Gaussian Blur", [&]() {
        return gaussian_blur(pixels, w, h, sigma);
    });

    // 2. Gradients & Sectors
    auto [gradients, sectors] = measure_time("Compute Gradients", [&]() {
        return compute_gradients(blurred_pixels, w, h);
    });

    // 3. Non-Maximum Suppression
    std::vector<float> suppressed_magnitudes = measure_time("Non-Max Suppression", [&]() {
        return non_maximum_suppression(gradients, sectors, w, h);
    });

    // 4. Double Thresholding
    std::vector<uint8_t> thresholded_pixels = measure_time("Double Thresholding", [&]() {
        return double_thresholding(suppressed_magnitudes, lower_threshold, upper_threshold);
    });

    // 5. Edge Hysteresis
    std::vector<uint8_t> final_edges = measure_time("Edge Hysteresis", [&]() {
        return edge_hysteresis(thresholded_pixels, w, h);
    });

    save_image_to_file(output_name, header, final_edges);

    return EXIT_SUCCESS;
}