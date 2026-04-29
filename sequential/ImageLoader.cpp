/**
 * @file ImageLoader.cpp
 * @brief Implementations for loading and saving TGA image files.
 * * Handles binary file I/O operations and format conversions (e.g., converting 
 * loaded RGB pixels to grayscale for edge detection).
 */

#include "ImageLoader.h"

#include <iostream>

std::pair<Header, std::vector<uint8_t>> load_image_from_file(const std::string & in_file_name) {
    Header header;
    std::ifstream in_file(in_file_name, std::ios::binary);
    in_file.read(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    in_file.read(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    in_file.read(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    in_file.read(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    in_file.read(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    in_file.read(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    in_file.read(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    in_file.read(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    in_file.read(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    in_file.read(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    in_file.read(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    in_file.read(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));

    size_t img_size = header.Width * header.Height;
    std::vector<uint8_t> pixels(img_size);

    int pixel_depth = header.PixelDepth / 8;
    
    // Read pixels and convert to grayscale using luminosity method
    for(size_t i = 0; i < img_size; ++i) {
        uint8_t r = 0, g = 0, b = 0;
        if(pixel_depth == 3) {
            in_file.read(reinterpret_cast<char*>(&b), sizeof(uint8_t));
            in_file.read(reinterpret_cast<char*>(&g), sizeof(uint8_t));
            in_file.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));
        } else if(pixel_depth == 1) {
            in_file.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));
            g = r;
            b = r;
        }

        pixels[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
    }

    return {header, pixels};
}

void save_image_to_file(const std::string & out_file_name, Header & header, std::vector<uint8_t> & pixels) {
    std::ofstream out_file(out_file_name, std::ios::binary);
    out_file.write(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    out_file.write(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    out_file.write(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    out_file.write(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    out_file.write(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    out_file.write(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    out_file.write(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    out_file.write(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    out_file.write(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    out_file.write(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    out_file.write(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    out_file.write(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));

    size_t img_size = pixels.size();
    for(size_t i = 0; i < img_size; ++i) {
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
    }
}

void save_image_to_file(const std::string & out_file_name, Header & header, std::vector<uint8_t> & pixels, std::vector<uint8_t> & original_pixels) {
    std::ofstream out_file(out_file_name, std::ios::binary);
    out_file.write(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    out_file.write(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    out_file.write(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    out_file.write(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    out_file.write(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    out_file.write(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    out_file.write(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    out_file.write(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    out_file.write(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    out_file.write(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    out_file.write(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    out_file.write(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));
    
    size_t img_size = pixels.size();
    for(size_t i = 0; i < img_size; ++i) {
        out_file.write(reinterpret_cast<char*>(&original_pixels[i]), sizeof(uint8_t));
        out_file.write(reinterpret_cast<char*>(&original_pixels[i]), sizeof(uint8_t));

        uint8_t pixel_value = (pixels[i] == 255) ? 255 : original_pixels[i];
        out_file.write(reinterpret_cast<char*>(&pixel_value), sizeof(uint8_t));
    }
}