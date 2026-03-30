#include <iostream>
#include <chrono>

#include "ImageLoader.h"
#include "Canny.h"

// zpracování chyb
// error handling
static void handle_error(cudaError_t err, const char* file, int line) {
    if (err != cudaSuccess) {
        printf("%s in %s at line %d\n", cudaGetErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}

#define HANDLE_ERROR(err) (handle_error(err, __FILE__, __LINE__))

int main(int argc, char const *argv[]) {
    if (argc < 5) {
        std::cout << "Usage: ./canny <image_path> <gamma> <lower_threshold> <upper_threshold>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_name = argv[1];
    std::string output_name1 = input_name;
    std::string output_name2 = input_name;

    output_name1.insert(input_name.size() - 4, "_edges" );
    output_name2.insert(input_name.size() - 4, "_only_edges" );

    float sigma = std::stof(argv[2]);
    float lower_threshold = std::stof(argv[3]);
    float upper_threshold = std::stof(argv[4]);

    auto [header, pixels] = load_image_from_file(input_name);
    int w = header.Width, h = header.Height;
    size_t img_size = w * h;

    uint8_t* original_pixels;
    uint8_t* blurred_pixels;
    float*   magnitudes = new float[img_size];
    uint8_t* sectors = new uint8_t[img_size];
    float*   suppressed_magnitudes = new float[img_size];
    uint8_t* edges = new uint8_t[img_size];

    int kernel_size = 5;
    float* gauss_kernel = new float[kernel_size * kernel_size];
    create_gaussian_kernel(gauss_kernel, 2, sigma);
    float* gauss_kernel_gpu;

    cudaDeviceProp prop;
    int which_device;

    HANDLE_ERROR(cudaGetDevice(&which_device));
    HANDLE_ERROR(cudaGetDeviceProperties(&prop, which_device));

    HANDLE_ERROR(cudaMallocManaged((void**)&original_pixels, img_size * sizeof(uint8_t)));
    std::copy(pixels.begin(), pixels.end(), original_pixels);

    HANDLE_ERROR(cudaMallocManaged((void**)&blurred_pixels, img_size * sizeof(uint8_t)));

    HANDLE_ERROR(cudaMalloc((void**)&gauss_kernel_gpu, kernel_size * kernel_size * sizeof(float)));
    HANDLE_ERROR(cudaMemcpy(gauss_kernel_gpu, gauss_kernel, kernel_size * kernel_size * sizeof(float), cudaMemcpyHostToDevice));

    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((w + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                   (h + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // 1. Gaussian Blur
    gaussian_blur<<<numBlocks, threadsPerBlock>>>(blurred_pixels, original_pixels, w, h, gauss_kernel_gpu, 2);
    std::cout << "Gaussian Blur done" << std::endl;

    HANDLE_ERROR(cudaDeviceSynchronize());

    // 2. Gradients & Sectors
    compute_gradients(magnitudes, sectors, blurred_pixels, w, h);
    std::cout << "Gradients done" << std::endl;

    // 3. Non-Maximum Suppression
    non_maximum_suppression(suppressed_magnitudes, magnitudes, sectors, w, h);
    std::cout << "Non-Maximum Suppression done" << std::endl;

    // 4. Double Thresholding
    double_thresholding(edges, suppressed_magnitudes, img_size, lower_threshold, upper_threshold);
    std::cout << "Double Thresholding done" << std::endl;

    // 5. Edge Hysteresis
    edge_hysteresis(edges, w, h);
    std::cout << "Hysteresis done" << std::endl;

    save_image_to_file(output_name1, header, edges, pixels);
    save_image_to_file(output_name2, header, edges);

    HANDLE_ERROR(cudaFree(original_pixels));
    HANDLE_ERROR(cudaFree(blurred_pixels));
    HANDLE_ERROR(cudaFree(gauss_kernel_gpu));
    
    delete[] gauss_kernel;
    delete[] magnitudes;
    delete[] sectors;
    delete[] suppressed_magnitudes;
    delete[] edges;

    return EXIT_SUCCESS;
}