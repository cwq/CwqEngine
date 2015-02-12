#ifndef IMAGE_H
#define IMAGE_H
#include <GLES2/gl2.h>

class Image
{
public:
    Image();
    ~Image();

    bool initWithFileName(const char* filename);
    bool initWithFileData(const unsigned char* data, int dataLen);

    void setPixels(void *ps, int length);
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
