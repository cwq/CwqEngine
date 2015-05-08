#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include "Image.h"

class Resource;

class Texture2D
{
public:
    Texture2D();
    ~Texture2D();

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

    void increaseRef();
    void decreaseRef();
    int getRef() const;

    static void initMaxTextureSize();
    static GLint getMaxTextureSize();

private:
    //Depending on GPU
    static GLint maxTextureSize;

    int mWidth;
    int mHeight;
    GLuint mTextureID;
    GLenum mTextureFormat;

    int mRef;
};

#endif // !TEXTURE2D_H
