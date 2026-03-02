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
        int kCenter = kernelSize / 2;

        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                
                float cr = 0, cg = 0, cb = 0;
                for (int ki = 0; ki < kernelSize; ++ki) {
                    for (int kj = 0; kj < kernelSize; ++kj) {
                        int ni = std::clamp(i + (ki - kCenter), 0, h - 1);
                        int nj = std::clamp(j + (kj - kCenter), 0, w - 1);

                        const Pixel & p = img[ni * w + nj];
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

    inline std::vector<float> createGaussianKernel(int k, float sigma) {
        int size = 2 * k + 1;
        std::vector<float> kernel(size * size);
        float sum = 0.0f;
        float twoSigmaSq = 2.0f * sigma * sigma;
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                float x = i - k;
                float y = j - k;
                
                float exponent = -(x * x + y * y) / twoSigmaSq;
                float value = std::exp(exponent);

                kernel[i * size + j] = value;
                sum += value;
            }
        }

        for (float & weight : kernel)
            weight /= sum;

        return kernel;
    }

    inline Image applyGauss(const Image & img, float sigma, int k = 2) {
        Image imgBlur = img;
        std::vector<float> kernel = ImageProcessing::createGaussianKernel(k, sigma);
        ImageProcessing::convolution(img, kernel, 2 * k + 1, imgBlur);
        return imgBlur;
    }

    inline std::vector<std::pair<float, uint8_t>> computeGradients(const Image & img) {
        int h = img.getHeight(), w = img.getWidth();

        std::vector<float> kernelX = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        std::vector<float> kernelY = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

        std::vector<float> gradX(w * h);
        std::vector<float> gradY(w * h);
        
        convolution(img, kernelX, 3, gradX);
        convolution(img, kernelY, 3, gradY);
        
        std::vector<std::pair<float, uint8_t>> gradients(w * h);
        for (int i = 0; i < w * h; ++i) {
            float mag = std::sqrt(gradX[i] * gradX[i] + gradY[i] * gradY[i]);
            float angle = std::atan2(gradY[i], gradX[i]) * 180.0f / M_PI;

            if (angle < 0) angle += 180.0f;

            uint8_t sector = 0;
            if (angle >= 22.5 && angle < 67.5)
                sector = 135;  
            else if (angle >= 67.5 && angle < 112.5)
                sector = 90;
            else if (angle >= 112.5 && angle < 157.5)
                sector = 45;

            gradients[i] = {mag, sector};
        }

        return gradients;
    }

    inline Image nonMaximumSuppression(const Image & img, const std::vector<std::pair<float, uint8_t>> & gradients,
                                       int lowThreshold, int highThreshold) {
        Image imgSuppressed(img.getHeader());
        int h = img.getHeight(), w = img.getWidth();

        for (int i = 1; i < h - 1; ++i) {
            for (int j = 1; j < w - 1; ++j) {
                int idx = i * w + j;
                float currentMag = gradients[idx].first;
                int sector = gradients[idx].second;

                float mag1 = 0, mag2 = 0;
                if (sector == 0) {
                    mag1 = gradients[i * w + (j + 1)].first; 
                    mag2 = gradients[i * w + (j - 1)].first; 
                } else if (sector == 90) {
                    mag1 = gradients[(i + 1) * w + j].first; 
                    mag2 = gradients[(i - 1) * w + j].first; 
                } else if (sector == 135) {
                    mag1 = gradients[(i - 1) * w + (j - 1)].first; 
                    mag2 = gradients[(i + 1) * w + (j + 1)].first; 
                } else if (sector == 45) {
                    mag1 = gradients[(i - 1) * w + (j + 1)].first; 
                    mag2 = gradients[(i + 1) * w + (j - 1)].first; 
                }

                uint8_t val = 0;
                if (currentMag > mag1 && currentMag > mag2) {
                    if (currentMag > highThreshold) {
                        val = 255; // Strong Edge
                    } else if (currentMag > lowThreshold) {
                        val = 128; // Weak Edge (Potential Edge)
                    } else {
                        val = 0;
                    }
                } 
                imgSuppressed[idx] = Pixel(val);
            }
        }
        return imgSuppressed;
    }

    inline Image applyHysteresis(const Image & img) {
        Image finalEdges = img;
        int h = img.getHeight(), w = img.getWidth();
        
        std::vector<std::pair<int, int>> edgesToProcess;
        for (int i = 1; i < h - 1; ++i) {
            for (int j = 1; j < w - 1; ++j) {
                if (finalEdges[i * w + j].r == 255) {
                    edgesToProcess.push_back({i, j});
                }
            }
        }

        while (!edgesToProcess.empty()) {
            auto [r, c] = edgesToProcess.back();
            edgesToProcess.pop_back();

            for (int ni = -1; ni <= 1; ++ni) {
                for (int nj = -1; nj <= 1; ++nj) {
                    int nr = r + ni;
                    int nc = c + nj;

                    if (nr >= 0 && nr < h && nc >= 0 && nc < w) {
                        int nIdx = nr * w + nc;

                        if (finalEdges[nIdx].r == 128) { 
                            finalEdges[nIdx] = Pixel(255); 
                            edgesToProcess.push_back({nr, nc}); 
                        }
                    }
                }
            }
        }

        for (int i = 0; i < h * w; ++i) {
            if (finalEdges[i].r == 128) {
                finalEdges[i] = Pixel(0);
            }
        }

        return finalEdges;
    }
}