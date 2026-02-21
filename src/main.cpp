#include <iostream>

#include "Image.h"

int main(int argc, char const *argv[]) {

    Image img("lenna.tga");
    img.saveToFile("lenna-copy.tga");
    img.toGray();
    img.saveToFile("lenna-gray.tga");

    return EXIT_SUCCESS;
}