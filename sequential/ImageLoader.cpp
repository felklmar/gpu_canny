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
    for(size_t i = 0; i < img_size; ++i) {
        uint8_t b = 0, g = 0, r = 0;
        in_file.read(reinterpret_cast<char*>(&b), sizeof(uint8_t));
        in_file.read(reinterpret_cast<char*>(&g), sizeof(uint8_t));
        in_file.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));
        pixels[i] = 0.299f*r + 0.587f*g + 0.114f*b;
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

    for(size_t i = 0; i < pixels.size(); ++i) {
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        out_file.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
    }
}
