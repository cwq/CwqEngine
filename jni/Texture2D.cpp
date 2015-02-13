#include "Texture2D.h"
#include "LogHelper.h"

#define TEXTURE_ID_INVALID 0

Textur2D::Textur2D()
{
    mTextureID = TEXTURE_ID_INVALID;
}

Textur2D::~Textur2D()
{
    mTextureID = TEXTURE_ID_INVALID;
}

bool Textur2D::load(const char* filename)
{
    Image image;
    image.initWithFileName(filename);
    return load(image);
}

bool Textur2D::load(const unsigned char* fileData, size_t dataLen)
{
    Image image;
    image.initWithFileData(fileData, dataLen);
    return load(image);
}

bool Textur2D::load(Image image)
{
    if(mWidth != image.getWidth() || mHeight != image.getHeight()
            || mTextureFormat != image.getFormat())
    {
        unLoad();
        GLuint textures[1];
        glGenTextures(1, textures);
        if(textures[0] != 0 )
        {
            mTextureFormat = image.getFormat();
            setWidthAndHeight(image.getWidth(), image.getHeight());
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, mTextureFormat, mWidth, mHeight, 0, mTextureFormat, GL_UNSIGNED_BYTE, image.getPixels());
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            mTextureID = textures[0];
            return true;
        }
        else
        {
            LOGE("ERROR in loadTexture!");
            return false;
        }
    }
    else
    {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, mTextureFormat, GL_UNSIGNED_BYTE, image.getPixels());
        return true;
    }
}

void Textur2D::unLoad()
{
    if (mTextureID != TEXTURE_ID_INVALID)
    {
        glDeleteTextures(1, &mTextureID);
        mTextureID = TEXTURE_ID_INVALID;
    }
}

bool Textur2D::isLoaded()
{
    return mTextureID != TEXTURE_ID_INVALID;
}

void Textur2D::bind()
{
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}

int Textur2D::getWidth()
{
    return mWidth;
}

int Textur2D::getHeight()
{
    return mHeight;
}

void Textur2D::setWidthAndHeight(int pWidth, int pHeight)
{
    mWidth = pWidth;
    mHeight = pHeight;
}

GLuint Textur2D::getTextureID()
{
    return mTextureID;
}
