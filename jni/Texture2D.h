#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include "Image.h"

class Textur2D
{
public:
    Textur2D();
    ~Textur2D();

    bool load(const char* filename);
    bool load(const unsigned char* fileData, size_t dataLen);
    bool load(const Image& image);
    void unLoad();
    bool isLoaded() const;

    void bind() const;

    int getWidth() const;
    int getHeight() const;
    void setWidthAndHeight(int pWidth, int pHeight);

    GLuint getTextureID() const;

private:
    int mWidth;
    int mHeight;
    GLuint mTextureID;
    GLenum mTextureFormat;
};

#endif // !TEXTURE2D_H
