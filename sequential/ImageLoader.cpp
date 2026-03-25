#include "ImageLoader.h"
#include <iostream>

std::pair<Header, std::vector<uint8_t>> loadImageFromFile(const std::string & inFileName) {
    Header header;
    std::ifstream inFile(inFileName, std::ios::binary);
    inFile.read(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    inFile.read(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    inFile.read(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    inFile.read(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    inFile.read(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    inFile.read(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    inFile.read(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    inFile.read(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    inFile.read(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    inFile.read(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    inFile.read(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    inFile.read(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));

    size_t totalPixels = header.Width * header.Height;
    std::vector<uint8_t> pixels(totalPixels);
    for(size_t i = 0; i < totalPixels; ++i) {
        uint8_t r = 0, g = 0, b = 0;
        inFile.read(reinterpret_cast<char*>(&r), sizeof(uint8_t));
        inFile.read(reinterpret_cast<char*>(&g), sizeof(uint8_t));
        inFile.read(reinterpret_cast<char*>(&b), sizeof(uint8_t));
        pixels[i] = 0.299f*r + 0.587f*g + 0.114f*b;
    }    

    return {header, pixels};
}

void saveImageToFile(const std::string & outFileName, Header & header, std::vector<uint8_t> & pixels) {
    std::ofstream outFile(outFileName, std::ios::binary);
    outFile.write(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    outFile.write(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    outFile.write(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    outFile.write(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    outFile.write(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    outFile.write(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    outFile.write(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    outFile.write(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    outFile.write(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    outFile.write(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    outFile.write(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    outFile.write(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));

    for(size_t i = 0; i < pixels.size(); ++i) {
        outFile.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        outFile.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
        outFile.write(reinterpret_cast<char*>(&pixels[i]), sizeof(uint8_t));
    }
}
