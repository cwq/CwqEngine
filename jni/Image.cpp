#include "Image.h"

Image::Image()
{
    pixels = NULL;
}

Image::~Image()
{

}

bool Image::initWithFileName(const char* filename)
{
    return true;
}

bool Image::initWithFileData(const unsigned char* fileData, size_t dataLen)
{
    return true;
}

void Image::setPixels(void *ps, size_t size)
{

}

void* Image::getPixels()
{
    return pixels;
}

int Image::getWidth()
{
    return mWidth;
}

int Image::getHeight()
{
    return mHeight;
}

void Image::setWidthAndHeight(int pWidth, int pHeight)
{
    mWidth = pWidth;
    mHeight = pHeight;
}

GLenum Image::getFormat()
{
    return mFormat;
}

void Image::setFormat(GLenum format)
{
    mFormat = format;
}
