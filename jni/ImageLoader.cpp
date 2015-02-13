#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NEON
#include "stb_image.h"
#include "LogHelper.h"

bool ImageLoader::loadImageWithFileName(Image* image, const char* filename)
{
    FILE *fp = fopen(filename, "r");
    bool result = false;
    if(!fp)
    {
        size_t fileSize;
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        unsigned char* data = (unsigned char*) malloc(fileSize);
        if(data)
        {
            fileSize = fread(data, sizeof(unsigned char), fileSize, fp);
            if(fileSize > 0)
            {
                result = ImageLoader::loadImageWithFileData(image, data, fileSize);
            }
            else
            {
                LOGE("Read file:%s error", filename);
            }
            free(data);
        }
        else
        {
            LOGE("loadImageWithFileName malloc(%d) error", fileSize);
        }
        fclose(fp);
    }
    else
    {
        LOGE("Open file:%s error", filename);
    }
    return result;
}

bool ImageLoader::loadImageWithFileData(Image* image, const unsigned char* fileData, size_t dataLen)
{
    bool result = false;
    int w, h, n;
    unsigned char* pixels = stbi_load_from_memory(fileData, dataLen, &w, &h, &n, 0);
    if(pixels)
    {
        if(n != 3 && n != 4)
        {
            LOGE("%d byte per pixel", n);
        }
        else
        {
            GLenum format = GL_RGBA;
            if(n == 3)
                format = GL_RGB;

            if(image->initWithImageInfo(w, h, format))
            {
                image->setPixels(pixels);
                result = true;
            }
        }
        stbi_image_free(pixels);
    }
    else
    {
        LOGE("stbi_load_from_memory error");
    }
    return result;
}
