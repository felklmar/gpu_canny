#include <iostream>
#include <chrono>

#include "ImageLoader.h"
#include "Canny.h"

int main(int argc, char const *argv[]) {
    if (argc < 8) {
        std::cout << "Usage: ./canny <image_path> <gamma> "; 
        std::cout << "<lower_threshold> <upper_threshold> ";
        std::cout << "<2D_block_width> <2D_block_height> <1D_block_size>" << std::endl;
        return EXIT_FAILURE;
    }

    
    std::string input_name = argv[1];
    std::string output_name1 = input_name;
    std::string output_name2 = input_name;
    
    output_name1.insert(input_name.size() - 4, "_edges" );
    output_name2.insert(input_name.size() - 4, "_only_edges" );
    
    float sigma = std::stof(argv[2]);
    std::pair<float, float> thresholds = {std::stof(argv[3]), std::stof(argv[4])};
    std::pair<int, int> block_dimensions_2D = {std::stoi(argv[5]), std::stoi(argv[6])};
    int block_size_1D = std::stoi(argv[7]);
    
    std::cout << "(" << block_dimensions_2D.first << ", " << block_dimensions_2D.second << ") " << block_size_1D << " | ";

    auto [header, pixels] = load_image_from_file(input_name);

    std::vector<uint8_t> detected_edges = detect_edges(pixels, {header.Width, header.Height}, sigma, thresholds, block_dimensions_2D, block_size_1D);

    save_image_to_file(output_name1, header, detected_edges, pixels);
    save_image_to_file(output_name2, header, detected_edges);

    return EXIT_SUCCESS;
}