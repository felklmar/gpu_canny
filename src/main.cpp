#include <iostream>

#include "Image.h"
#include "ImageProcessing.h"

int main(int argc, char const *argv[]) {

    Image img("lenna.tga");
    Image imgGray = ImageProcessing::convertToGrayscale(img);
    imgGray.saveToFile("lenna-gray.tga");


    std::vector<float> gaussKernel5x5 = {
        0.0029690f, 0.0133062f, 0.0219382f, 0.0133062f, 0.0029690f,    
        0.0133062f, 0.0596343f, 0.0983203f, 0.0596343f, 0.0133062f,   
        0.0219382f, 0.0983203f, 0.1621030f, 0.0983203f, 0.0219382f,   
        0.0133062f, 0.0596343f, 0.0983203f, 0.0596343f, 0.0133062f,   
        0.0029690f, 0.0133062f, 0.0219382f, 0.0133062f, 0.0029690f, 
    };

    Image imgBlur = imgGray;
    ImageProcessing::convolution(imgGray, gaussKernel5x5, 5, imgBlur);
    imgBlur.saveToFile("lenna-blur.tga");

    Image imgSobel = ImageProcessing::applySobel(imgBlur);
    imgSobel.saveToFile("lenna-sobel.tga");

    return EXIT_SUCCESS;
}