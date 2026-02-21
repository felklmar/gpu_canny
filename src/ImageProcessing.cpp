#include "ImageProcessing.h"

#include <iostream>

namespace ImageProcessing {

Image convertToGrayscale(const Image & img) {
    Image imgGray = img;

    for(size_t i = 0; i < img.getWidth() * img.getWidth(); ++i)
        imgGray[i] = Pixel(0.299f * img[i].r + 0.587f * img[i].g + 0.114f * img[i].b);      

    return imgGray;
}

Image convolution(const Image & img, const std::vector<float> & kernel, size_t kernelSize) {
    Image imgBlur = img;

    size_t h = img.getHeight(), w = img.getWidth();
    size_t kernelCenter = (kernelSize - 1) / 2;

    for (size_t i = kernelCenter; i < h - kernelSize + 1; ++i){
        for (size_t j = kernelCenter; j < w - kernelSize + 1; ++j) {

            float cr = 0, cg = 0, cb = 0;
            for (size_t ki = 0; ki < kernelSize; ++ki) {
                for (size_t kj = 0; kj < kernelSize; ++kj) {
                    const Pixel & px = img[(i + ki) * w + (j + kj)];
                    float weight = kernel[ki * kernelSize + kj];
                    cr += px.r * weight;
                    cg += px.g * weight;
                    cb += px.b * weight;
                }
            }

            imgBlur[(i + kernelCenter) * w + (j + kernelCenter)] = Pixel(std::abs(cr), std::abs(cg), std::abs(cb));
        }
    }

    return imgBlur;
}

}
