#ifndef IMAGELOADER_H
#define IMAGELOADER_H
#include "Image.h"

class ImageLoader
{
public:
    static bool loadImageWithFileName(Image* image, const char* filename);
    static bool loadImageWithFileData(Image* image, const unsigned char* fileData, size_t dataLen);

};

#endif
