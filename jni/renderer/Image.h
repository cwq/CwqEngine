#ifndef IMAGE_H
#define IMAGE_H
#include "platform/CwqGL.h"

class Image
{
public:
    Image();
    ~Image();

    bool initWithFileName(const char* filename);
    bool initWithFileData(const unsigned char* fileData, size_t dataLen);
    bool initWithImageInfo(int pWidth, int pHeight, GLenum format);

    bool setPixels(const void *ps);
    void* getPixels() const;

    int getWidth() const;
    int getHeight() const;

    GLenum getFormat() const;

    bool isUpdated() const;
    void setUpdated(bool update);

private:
    bool mallocPixels(size_t size);
    void freePixels();

    void* pixels;
    size_t pSize;
    int mWidth;
    int mHeight;
    GLenum mFormat;

    //changed pixels, Texture2D need to reload
    bool updated;
};

#endif // !IMAGE_H
