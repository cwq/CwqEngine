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

private:
    bool mallocPixels(size_t size);
    void freePixels();

    void* pixels;
    size_t pSize;
    int mWidth;
    int mHeight;
    GLenum mFormat;
};

#endif // !IMAGE_H
