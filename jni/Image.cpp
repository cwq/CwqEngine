#include "Image.h"
#include <stdlib.h>
#include "LogHelper.h"

Image::Image()
{
    pixels = NULL;
}

Image::~Image()
{
    freePixels();
}

bool Image::initWithFileName(const char* filename)
{
    return true;
}

bool Image::initWithFileData(const unsigned char* fileData, size_t dataLen)
{
    return true;
}

bool Image::initWithImageInfo(int pWidth, int pHeight, GLenum format)
{
    mFormat = format;
    mWidth = pWidth;
    mHeight = pHeight;

    size_t size = mWidth * mHeight;
    if(format == GL_RGBA)
    {
        size *= 4;
    }
    else if(format == GL_RGB)
    {
        size *= 3;
    }
    else
    {
        LOGE("Invalid format: %d, format must be GL_RGB or GL_RGBA", format);
        return false;
    }

    if(!mallocPixels(size))
    {
        return false;
    }
    return true;
}

bool Image::setPixels(void *ps)
{
    if(pixels != NULL)
    {
        memcpy(pixels, ps, pSize);
        return true;
    }
    return false;
}

void* Image::getPixels() const
{
    return pixels;
}

int Image::getWidth() const
{
    return mWidth;
}

int Image::getHeight() const
{
    return mHeight;
}

GLenum Image::getFormat() const
{
    return mFormat;
}

bool Image::mallocPixels(size_t size)
{
    if(pixels != NULL)
    {
        //use last
        if(pSize >= size)
        {
            return true;
        }
        else
        {
            freePixels();
        }
    }
    //malloc
    pixels = malloc(size);
    if(pixels == NULL)
    {
        LOGE("Error malloc(%d)", size);
        return false;
    }
    return true;
}

void Image::freePixels()
{
    if(pixels != NULL)
    {
        free(pixels);
        pixels = NULL;
    }
}
