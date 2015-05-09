#include "ImageLoader.h"
#include "platform/Resource.h"
#include "base/LogHelper.h"

#define STB_IMAGE_IMPLEMENTATION
//HAVE_NEON is defined in Android.mk !
#ifdef HAVE_NEON
#define STBI_NEON
#endif // !HAVE_NEON
#include "stb_image.h"

bool ImageLoader::loadImageWithFileName(Image* image, const char* filename)
{
    bool result = false;
    Resource resource(filename);
    if(resource.open())
    {
        size_t fileSize = resource.getLength();
        unsigned char* data = (unsigned char*) malloc(fileSize);
        if(data)
        {
            if(resource.read(data, fileSize))
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
        resource.close();
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
    //req_comp=4, for 3 comp jpeg, use 4 is faster (neon)
    //for some png images, use comp will get wrong data (comp may be wrong)
    unsigned char* pixels = stbi_load_from_memory(fileData, dataLen, &w, &h, &n, 4);
    if(pixels)
    {
//        if(n != 3 && n != 4)
//        {
//            LOGE("%d byte per pixel", n);
//        }
//        else
//        {
            GLenum format = GL_RGBA;
            if(image->initWithImageInfo(w, h, format))
            {
                image->setPixels(pixels);
                result = true;
            }
//        }
        stbi_image_free(pixels);
    }
    else
    {
        LOGE("stbi_load_from_memory error");
    }
    return result;
}
