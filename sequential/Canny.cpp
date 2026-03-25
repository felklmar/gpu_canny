#include "Canny.h"

#include <math.h>
#include <algorithm>

std::vector<float> createGaussianKernel(int k, float sigma) {
    int size = 2 * k + 1;
    float sum = 0.0f;
    float twoSigmaSq = 2.0f * sigma * sigma;
    
    std::vector<float> kernel(size * size);
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

std::vector<uint8_t> gaussianBlur(const std::vector<uint8_t> & pixels, int w, int h, float sigma, int k) {
    std::vector<float> kernel = createGaussianKernel(k, sigma);
    int kernelSize = 2 * k + 1;
    
    std::vector<uint8_t> bluredPixels(w * h);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            float pixelValue = 0;
            for (int ki = 0; ki < kernelSize; ++ki) {
                for (int kj = 0; kj < kernelSize; ++kj) {
                    int ni = std::clamp(i + (ki - k), 0, h - 1);
                    int nj = std::clamp(j + (kj - k), 0, w - 1);
                    pixelValue += pixels[ni * w + nj] * kernel[ki * kernelSize + kj];
                }
            }

            bluredPixels[i * w + j] = pixelValue;
        }
    }

    return bluredPixels;
}

std::pair<std::vector<float>, std::vector<uint8_t>> computeGradients(const std::vector<uint8_t> & pixels, int w, int h) {
    std::vector<float> kernelX = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    std::vector<float> kernelY = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    
    int k = 1;
    int kernelSize = 2 * k + 1;

    std::vector<float> magnitudes(w * h);
    std::vector<uint8_t> sectors(w * h);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            float convX = 0, convY = 0;
            for (int ki = 0; ki < kernelSize; ++ki) {
                for (int kj = 0; kj < kernelSize; ++kj) {
                    int ni = std::clamp(i + (ki - k), 0, h - 1);
                    int nj = std::clamp(j + (kj - k), 0, w - 1);                       
                    convX += pixels[ni * w + nj] * kernelX[ki * kernelSize + kj];
                    convY += pixels[ni * w + nj] * kernelY[ki * kernelSize + kj];
                }
            }

            float mag = std::sqrt(convX * convX + convY * convY);
            float angle = std::atan2(convY, convX) * 180.0f / M_PI;
            if (angle < 0) angle += 180.0f;

            uint8_t sector = 0;
            if (angle > 22.5 && angle <= 67.5) {
                sector = 45;
            } else if (angle > 67.5 && angle <= 112.5) {
                sector = 90;
            } else if (angle > 112.5 && angle <= 157.5) {
                sector = 135; 
            }

            magnitudes[i * w + j] = mag;
            sectors[i * w + j] = sector;
        }
    }

    return {magnitudes, sectors};
}

std::vector<float> nonMaximumSuppression(const std::vector<float> & gradients, const std::vector<uint8_t> & sectors, int w, int h) {
    std::vector<float> suppressedEdges(w * h, 0.0f);
    for (int i = 1; i < h - 1; ++i) {
        for (int j = 1; j < w - 1; ++j) {
            int idx = i * w + j;
            float grad = gradients[idx];
            
            if (grad < 1e-5f) continue;

            bool isLocalMax = true;
            switch (sectors[idx]) {
            case 0:
                isLocalMax = !(gradients[idx - 1] >= grad || gradients[idx + 1] > grad);          
                break;
            case 45:
                isLocalMax = !(gradients[idx - (w + 1)] >= grad || gradients[idx + (w + 1)] > grad);
                break;
            case 90:
                isLocalMax = !(gradients[idx - w] >= grad || gradients[idx + w] > grad);
                break;
            case 135:
                isLocalMax = !(gradients[idx - (w - 1)] >= grad || gradients[idx + (w - 1)] > grad);
                break;
            default:
                isLocalMax = false;
                break;
            }
            
            if (isLocalMax)
                suppressedEdges[idx] = grad;
        }
    }

    return suppressedEdges;
}

std::vector<uint8_t> doubleThresholding(const std::vector<float> & suppressed, float lowerPercent, float upperPercent) {
    size_t totalPixels = suppressed.size();
    std::vector<uint8_t> result(totalPixels, 0);

    auto maxIt = std::max_element(suppressed.begin(), suppressed.end());
    float maxGrad = (maxIt != suppressed.end()) ? *maxIt : 0.0f;

    if (maxGrad < 1e-5f) return result; 

    float lowerThreshold = maxGrad * lowerPercent;
    float upperThreshold = maxGrad * upperPercent;

    for (size_t i = 0; i < totalPixels; ++i) {
        float grad = suppressed[i];
        
        if (grad >= upperThreshold)
            result[i] = 255;
        else if (grad >= lowerThreshold)
            result[i] = 128;
    }

    return result;
}

std::vector<uint8_t> edgeHysteresis(const std::vector<uint8_t> & pixels, int w, int h) {
    std::vector<uint8_t> finalEdges = pixels;
    
    std::vector<int> edgesToProcess;
    for (int i = 1; i < h - 1; ++i) {
        for (int j = 1; j < w - 1; ++j) {
            int idx = i * w + j;
            if (finalEdges[idx] == 255)
                edgesToProcess.push_back(idx);
        }
    }

    while (!edgesToProcess.empty()) {
        int idx = edgesToProcess.back();
        edgesToProcess.pop_back();

        int r = idx / w;
        int c = idx % w;

        int neighbors[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };

        for (int i = 0; i < 8; ++i) {
            int nr = r + neighbors[i][0];
            int nc = c + neighbors[i][1];

            if (nr >= 0 && nr < h && nc >= 0 && nc < w) {
                int nIdx = nr * w + nc;
                if (finalEdges[nIdx] == 128) {
                    finalEdges[nIdx] = 255;
                    edgesToProcess.push_back(nIdx);
                }
            }
        }
    }

    int totalPixels = w * h;
    for (int i = 0; i < totalPixels; ++i) {
        if (finalEdges[i] == 128)
            finalEdges[i] = 0;
    }

    return finalEdges;
}