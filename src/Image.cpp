#include "Image.h"
#include <iostream>

Pixel::Pixel() : r(0), g(0), b(0) {}
Pixel::Pixel(uint8_t val) : r(val), g(val), b(val) {}
Pixel::Pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {} 

Image::Image(const std::string & inputFileName) {
    std::ifstream inputFile(inputFileName, std::ios::binary);
    m_Header = readHeader(inputFile);
    m_Pixels = readPixels(inputFile, m_Header.Width, m_Header.Height);
    inputFile.close();  
}

Header Image::readHeader(std::ifstream & file) {
    Header header;

    file.read(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    file.read(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    file.read(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    file.read(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    file.read(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    file.read(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    file.read(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    file.read(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    file.read(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    file.read(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    file.read(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    file.read(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));

    return header;
}

void Image::writeHeader(std::ofstream & file, Header header) {
    file.write(reinterpret_cast<char*>(&header.IDLength), sizeof(header.IDLength));
    file.write(reinterpret_cast<char*>(&header.ColorMapType), sizeof(header.ColorMapType));
    file.write(reinterpret_cast<char*>(&header.ImageType), sizeof(header.ImageType));
    file.write(reinterpret_cast<char*>(&header.CMapStart), sizeof(header.CMapStart));
    file.write(reinterpret_cast<char*>(&header.CMapLength), sizeof(header.CMapLength));
    file.write(reinterpret_cast<char*>(&header.CMapDepth), sizeof(header.CMapDepth));
    file.write(reinterpret_cast<char*>(&header.XOffset), sizeof(header.XOffset));
    file.write(reinterpret_cast<char*>(&header.YOffset), sizeof(header.YOffset));
    file.write(reinterpret_cast<char*>(&header.Width), sizeof(header.Width));
    file.write(reinterpret_cast<char*>(&header.Height), sizeof(header.Height));
    file.write(reinterpret_cast<char*>(&header.PixelDepth), sizeof(header.PixelDepth));
    file.write(reinterpret_cast<char*>(&header.ImageDescriptor), sizeof(header.ImageDescriptor));
}

void Image::printHeader() {
    std::cout << (int)m_Header.IDLength         << "\n";
    std::cout << (int)m_Header.ColorMapType     << "\n";
    std::cout << (int)m_Header.ImageType        << "\n";
    std::cout << (int)m_Header.CMapStart        << "\n";
    std::cout << (int)m_Header.CMapLength       << "\n";
    std::cout << (int)m_Header.CMapDepth        << "\n";
    std::cout << (int)m_Header.XOffset          << "\n";
    std::cout << (int)m_Header.YOffset          << "\n";
    std::cout << (int)m_Header.Width            << "\n";
    std::cout << (int)m_Header.Height           << "\n";
    std::cout << (int)m_Header.PixelDepth       << "\n";
    std::cout << (int)m_Header.ImageDescriptor  << std::endl;
}

std::vector<Pixel> Image::readPixels(std::ifstream & file, uint16_t width, uint16_t height) {
    std::vector<Pixel> pixels(width * height);
    for(size_t i = 0; i < width * height; ++i) {
        file.read(reinterpret_cast<char*>(&pixels[i].r), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&pixels[i].g), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&pixels[i].b), sizeof(uint8_t));
    }
    return pixels;
}

void Image::writePixels(std::ofstream & file, std::vector<Pixel> pixels, uint16_t width, uint16_t height) {
    for(size_t i = 0; i < width * height; ++i) {
        file.write(reinterpret_cast<char*>(&pixels[i].r), sizeof(uint8_t));
        file.write(reinterpret_cast<char*>(&pixels[i].g), sizeof(uint8_t));
        file.write(reinterpret_cast<char*>(&pixels[i].b), sizeof(uint8_t));
    }
}

void Image::saveToFile(const std::string & outputFileName) {
    std::ofstream outputFile(outputFileName, std::ios::binary);
    writeHeader(outputFile, m_Header);
    writePixels(outputFile, m_Pixels, m_Header.Width, m_Header.Height);
    outputFile.close();
}

uint16_t Image::getWidth() const { return m_Header.Width; }
uint16_t Image::getHeight() const { return m_Header.Height; }

Pixel& Image::operator [](int idx) { return m_Pixels[idx]; }
Pixel  Image::operator [](int idx) const { return m_Pixels[idx]; }