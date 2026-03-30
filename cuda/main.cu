#include <iostream>
#include <chrono>

#include "ImageLoader.h"
#include "Canny.h"

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

    std::vector<uint8_t> detected_edges = detect_edges(pixels, header.Width, header.Height, sigma, lower_threshold, upper_threshold);

    save_image_to_file(output_name1, header, detected_edges, pixels);
    save_image_to_file(output_name2, header, detected_edges);

    return EXIT_SUCCESS;
}