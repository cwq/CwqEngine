#ifndef IMAGE_H
#define IMAGE_H
#include <GLES2/gl2.h>

class Image
{
public:
    Image();
    ~Image();

    bool initWithFileName(const char* filename);
    bool initWithFileData(const unsigned char* fileData, size_t dataLen);

    void setPixels(void *ps, size_t size);
    void* getPixels();

    int getWidth();
    int getHeight();
    void setWidthAndHeight(int pWidth, int pHeight);

    GLenum getFormat();
    void setFormat(GLenum format);

private:
    void* pixels;
    int mWidth;
    int mHeight;
    GLenum mFormat;
};

#endif // !IMAGE_H
