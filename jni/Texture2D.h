#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include "Image.h"

class Textur2D
{
public:
    Textur2D();
    ~Textur2D();

    bool load(const char* filename);
    bool load(const unsigned char* data, int dataLen);
    bool load(Image image);
    void unLoad();
    bool isLoaded();

    void bind();

    int getWidth();
    int getHeight();
    void setWidthAndHeight(int pWidth, int pHeight);

    GLuint getTextureID();

private:
    int mWidth;
    int mHeight;
    GLuint mTextureID;
    GLenum mTextureFormat;
};

#endif // !TEXTURE2D_H
