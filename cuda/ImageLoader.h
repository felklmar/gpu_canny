#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

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

std::pair<Header, std::vector<uint8_t>> load_image_from_file(const std::string & in_file_name);
void save_image_to_file(const std::string & outFileName, Header & header, uint8_t* pixels);
void save_image_to_file(const std::string & out_file_name, Header & header, uint8_t* pixels, std::vector<uint8_t> & original_pixels);