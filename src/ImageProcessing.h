#pragma once

#include "Image.h"

#include <algorithm>
#include <math.h>

namespace ImageProcessing {
    inline Image convertToGrayscale(const Image & img) {
        Image imgGray = img;

        for(size_t i = 0; i < img.getWidth() * img.getHeight(); ++i)
            imgGray[i] = Pixel(0.299f * img[i].r + 0.587f * img[i].g + 0.114f * img[i].b);      

        return imgGray;
    }
    
    inline void storeResult(Image & out, int idx, float r, float g, float b) {
        out[idx] = Pixel(
            std::clamp(r, 0.0f, 255.0f),
            std::clamp(g, 0.0f, 255.0f),
            std::clamp(b, 0.0f, 255.0f)
        );
    }

    inline void storeResult(std::vector<float> & out, int idx, float r, float g, float b) {
        out[idx] = r; 
    }

    template <typename T>
    void convolution(const Image & img, const std::vector<float> & kernel, int kernelSize, T & output) {
        int h = img.getHeight(), w = img.getWidth();
        int kCenter = (kernelSize - 1) / 2;

        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                
                float cr = 0, cg = 0, cb = 0;
                for (int ki = 0; ki < kernelSize; ++ki) {
                    for (int kj = 0; kj < kernelSize; ++kj) {
                        int ni = std::clamp(i + (ki - kCenter), 0, h - 1);
                        int nj = std::clamp(j + (kj - kCenter), 0, w - 1);

                        const auto & p = img[ni * w + nj];
                        float weight = kernel[ki * kernelSize + kj];
                        
                        cr += p.r * weight;
                        cg += p.g * weight;
                        cb += p.b * weight;
                    }
                }
                storeResult(output, i * w + j, cr, cg, cb);
            }
        }
    }

    inline Image applySobel( const Image & img ) {
        int h = img.getHeight(), w = img.getWidth();

        std::vector<float> kernelX = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        std::vector<float> kernelY = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

        std::vector<float> gradX(w * h);
        std::vector<float> gradY(w * h);
        
        convolution(img, kernelX, 3, gradX);
        convolution(img, kernelY, 3, gradY);

        Image result = img;
        for (int i = 0; i < w * h; ++i) {
            float magnitude = std::sqrt(gradX[i] * gradX[i] + gradY[i] * gradY[i]);
            result[i] = Pixel(static_cast<uint8_t>(std::clamp(magnitude, 0.0f, 255.0f)));
        }

        return result;
    }
}