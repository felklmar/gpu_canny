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
        std::cout << "Usage: ./canny <image_path> <gamma> <lower_threshold> <upper_threshold>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_name = argv[1];
    std::string output_name1 = input_name;
    std::string output_name2 = input_name;

    output_name1.insert(input_name.size() - 4, "_edges" );
    output_name2.insert(input_name.size() - 4, "_only_edges" );
    
    float sigma = std::stof(argv[2]);
    float lower_threshold = std::stof(argv[3]);
    float upper_threshold = std::stof(argv[4]);

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

    save_image_to_file(output_name1, header, final_edges, pixels);
    save_image_to_file(output_name2, header, final_edges);

    return EXIT_SUCCESS;
}