#include <iostream>

#include "Image.h"
#include "ImageProcessing.h"

int main(int argc, char const *argv[]) {

    Image img("lenna.tga");
    Image imgGray = ImageProcessing::convertToGrayscale(img);
    imgGray.saveToFile("lenna-gray.tga");
    
    Image imgBlur = ImageProcessing::applyGauss(imgGray, 2, 2);
    imgBlur.saveToFile("lenna-blur.tga");
    
    std::vector<std::pair<float,uint8_t>> gradients = ImageProcessing::computeGradients(imgBlur);
    
    Image imgSuppressed = ImageProcessing::nonMaximumSuppression(imgBlur, gradients, 40, 100);
    imgSuppressed.saveToFile("lenna-suppressed.tga");

    Image imgFinal = ImageProcessing::applyHysteresis(imgSuppressed);
    imgFinal.saveToFile("lenna-final.tga");

    return EXIT_SUCCESS;
}