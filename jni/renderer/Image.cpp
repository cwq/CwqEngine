#include "Image.h"
#include <stdlib.h>
#include <string.h>
#include "ImageLoader.h"
#include "base/LogHelper.h"

Image::Image()
{
    pixels = NULL;
    pSize = 0;
    mWidth = mHeight = 0;
    mFormat = GL_INVALID_VALUE;
    updated = false;
}

Image::~Image()
{
    freePixels();
}

bool Image::initWithFileName(const char* filename)
{
    return ImageLoader::loadImageWithFileName(this, filename);
}

bool Image::initWithFileData(const unsigned char* fileData, size_t dataLen)
{
    return ImageLoader::loadImageWithFileData(this, fileData, dataLen);
}

bool Image::initWithImageInfo(int pWidth, int pHeight, GLenum format)
{
    mFormat = format;
    mWidth = pWidth;
    mHeight = pHeight;

    size_t size;
    if(format == GL_RGBA)
    {
        lineSize = mWidth * 4;
        size = lineSize * mHeight;
    }
    else if(format == GL_RGB)
    {
        lineSize = mWidth * 3;
        int ret = lineSize % 4;
        if (ret) {
            lineSize += (4 - ret);
        }
        size = lineSize * mHeight;
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
    pSize = size;
    return true;
}

bool Image::setPixels(const void *ps)
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

int Image::getLineSize() const
{
    return lineSize;
}

GLenum Image::getFormat() const
{
    return mFormat;
}

bool Image::isUpdated() const
{
    return updated;
}

void Image::setUpdated(bool update)
{
    updated = update;
}

bool Image::mallocPixels(size_t size)
{
    if(pixels != NULL)
    {
        //if size is bigger than last, realloc
        if(pSize < size)
        {
            void* p = realloc(pixels, size);
            if(p == NULL)
            {
                LOGE("Error realloc(%p, %d)", pixels, size);
                return false;
            }
            else
            {
                pixels = p;
            }
        }
    }
    else
    {
        //malloc
        pixels = malloc(size);
        if(pixels == NULL)
        {
            LOGE("Error malloc(%d)", size);
            return false;
        }
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
