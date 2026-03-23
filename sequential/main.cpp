#include <iostream>

#include "Image.h"
#include "ImageProcessing.h"

int main(int argc, char const *argv[]) {
    if (argc < 5) {
        std::cout << "Usage: ./canny <input_picture> <output_name> <gamma> <lower_threshold> <upper_threshold>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputName = argv[1];
    std::string outputName = argv[2];

    float sigma = std::stof(argv[3]);
    int lowerThreshold = std::stoi(argv[4]);
    int upperThreshold = std::stoi(argv[5]);

    Image img(inputName);
    Image imgGray = ImageProcessing::convertToGrayscale(img);
    //imgGray.saveToFile("gray.tga");
   
    Image imgBlur = ImageProcessing::applyGauss(imgGray, sigma);
    //imgBlur.saveToFile("blur.tga");
    
    std::vector<std::pair<float,uint8_t>> gradients = ImageProcessing::computeGradients(imgBlur);
    Image imgSuppressed = ImageProcessing::nonMaximumSuppression(imgBlur, gradients, lowerThreshold, upperThreshold);
    //imgSuppressed.saveToFile("suppressed.tga");

    Image imgFinal = ImageProcessing::applyHysteresis(imgSuppressed);
    imgFinal.saveToFile(outputName);

    return EXIT_SUCCESS;
}