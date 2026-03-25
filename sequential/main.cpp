#include <iostream>

#include "ImageLoader.h"
#include "Canny.h"

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

    std::vector<uint8_t> bluredPixels = gaussianBlur(pixels, w, h, sigma);
    auto [gradients, sectors] = computeGradients(bluredPixels, w, h);
    std::vector<float> suppressedPixels = nonMaximumSuppression(gradients, sectors, w, h);
    std::vector<uint8_t> thresholdedPixels = doubleThresholding(suppressedPixels, lowerThreshold, upperThreshold);
    std::vector<uint8_t> finalEdges = edgeHysteresis(thresholdedPixels, w, h);

    saveImageToFile(outputName, header, finalEdges);

    return EXIT_SUCCESS;
}