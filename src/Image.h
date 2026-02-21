#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>

struct Header {
    uint8_t  IDLength;          //  0 (1 byte)
    uint8_t  ColorMapType;      //  1 (1 byte)
    uint8_t  ImageType;         //  2 (1 byte)
    uint16_t CMapStart;         //  3 (2 byte)
    uint16_t CMapLength;        //  5 (2 byte)
    uint8_t  CMapDepth;         //  7 (1 byte)
    uint16_t XOffset;           //  8 (2 byte)
    uint16_t YOffset;           // 10 (2 byte)
    uint16_t Width;             // 12 (2 byte)
    uint16_t Height;            // 14 (2 byte)
    uint8_t  PixelDepth;        // 16 (1 byte)
    uint8_t  ImageDescriptor;   // 17 (1 byte)
};

struct Pixel {
    uint8_t r, g, b;
    Pixel();
    Pixel(uint8_t val);
    Pixel(uint8_t r, uint8_t g, uint8_t b);
};

class Image {
    private:
        Header m_Header;
        std::vector<Pixel> m_Pixels;

        Header readHeader(std::ifstream & file);
        void writeHeader(std::ofstream & file, Header header);
        void printHeader();

        std::vector<Pixel> readPixels(std::ifstream & file, uint16_t width, uint16_t height);
        void writePixels(std::ofstream & file, std::vector<Pixel> pixels, uint16_t width, uint16_t height);
    public:
        Image() = default;
        Image(const std::string & inputFileName);
        void toGray();
        void saveToFile(const std::string & outputFileName);
};