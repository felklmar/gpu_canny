#pragma once

#include "Image.h"

namespace ImageProcessing {
    Image convertToGrayscale(const Image & img);
    Image convolution(const Image & img, const std::vector<float> & kernel, size_t kernelSize);
}