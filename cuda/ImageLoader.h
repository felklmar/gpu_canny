/**
 * @file ImageLoader.h
 * @brief Utilities for reading and writing TGA (Truevision Targa) image files.
 */

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <utility>

/**
 * @struct Header
 * @brief Represents the standard 18-byte header of a TGA image file.
 */
struct Header {
    uint8_t  IDLength;          ///< Size of Image ID field (1 byte)
    uint8_t  ColorMapType;      ///< Color map type (1 byte)
    uint8_t  ImageType;         ///< Image type code (1 byte)
    uint16_t CMapStart;         ///< Color map origin (2 bytes)
    uint16_t CMapLength;        ///< Color map length (2 bytes)
    uint8_t  CMapDepth;         ///< Depth of color map entries (1 byte)
    uint16_t XOffset;           ///< X origin of image (2 bytes)
    uint16_t YOffset;           ///< Y origin of image (2 bytes)
    uint16_t Width;             ///< Width of image (2 bytes)
    uint16_t Height;            ///< Height of image (2 bytes)
    uint8_t  PixelDepth;        ///< Image pixel size (1 byte)
    uint8_t  ImageDescriptor;   ///< Image descriptor byte (1 byte)
};

/**
 * @brief Loads a TGA image from disk and converts it to a flattened grayscale pixel array.
 * * @param in_file_name The path to the input TGA file.
 * @return std::pair<Header, std::vector<uint8_t>> A pair containing the parsed TGA header and the grayscale pixel data.
 */
std::pair<Header, std::vector<uint8_t>> load_image_from_file(const std::string & in_file_name);

/**
 * @brief Saves a grayscale pixel array to a TGA file.
 * * @param out_file_name The path to the output TGA file.
 * @param header The original TGA header to maintain image metadata.
 * @param pixels The grayscale pixel data to save.
 */
void save_image_to_file(const std::string & out_file_name, Header & header, std::vector<uint8_t> & pixels);

/**
 * @brief Saves a grayscale edge map overlaid on top of the original color/grayscale image.
 * * @param out_file_name The path to the output TGA file.
 * @param header The original TGA header.
 * @param pixels The binary edge map data (0 or 255).
 * @param original_pixels The original image data used as the background.
 */
void save_image_to_file(const std::string & out_file_name, Header & header, std::vector<uint8_t> & pixels, std::vector<uint8_t> & original_pixels);