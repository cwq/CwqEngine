#include "Texture2D.h"
#include "base/LogHelper.h"

GLint Texture2D::maxTextureSize = 0;

#define TEXTURE_ID_INVALID 0

Texture2D::Texture2D()
{
    mWidth = mHeight = 0;
    mTextureID = TEXTURE_ID_INVALID;
    mTextureFormat = GL_INVALID_VALUE;
    mRef = 1;
}

Texture2D::~Texture2D()
{
    mTextureID = TEXTURE_ID_INVALID;
    mRef = 0;
}

bool Texture2D::load(const char* filename)
{
    Image image;
    image.initWithFileName(filename);
    return load(image);
}

bool Texture2D::load(const unsigned char* fileData, size_t dataLen)
{
    Image image;
    image.initWithFileData(fileData, dataLen);
    return load(image);
}

bool Texture2D::load(const Image& image)
{
    //null pixels
    if(image.getPixels() == NULL)
    {
        LOGE("Image null pixels!");
        return false;
    }

    int imageWidth = image.getWidth();
    int imageHeight = image.getHeight();

    if (imageWidth > maxTextureSize || imageHeight > maxTextureSize)
    {
        LOGE("Image (%d x %d) is bigger than the supported (%d x %d)", imageWidth, imageHeight, maxTextureSize, maxTextureSize);
        return false;
    }

    if(mWidth != imageWidth || mHeight != imageHeight
            || mTextureFormat != image.getFormat() || mTextureID == TEXTURE_ID_INVALID)
    {
        unLoad();
        GLuint textures[1];
        glGenTextures(1, textures);
        if(textures[0] != 0 )
        {
            mTextureFormat = image.getFormat();
            setWidthAndHeight(imageWidth, imageHeight);
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

void Texture2D::unLoad()
{
    if (mTextureID != TEXTURE_ID_INVALID)
    {
        glDeleteTextures(1, &mTextureID);
        mTextureID = TEXTURE_ID_INVALID;
    }
}

bool Texture2D::isLoaded() const
{
    return mTextureID != TEXTURE_ID_INVALID;
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}

int Texture2D::getWidth() const
{
    return mWidth;
}

int Texture2D::getHeight() const
{
    return mHeight;
}

void Texture2D::setWidthAndHeight(int pWidth, int pHeight)
{
    mWidth = pWidth;
    mHeight = pHeight;
}

GLuint Texture2D::getTextureID() const
{
    return mTextureID;
}

void Texture2D::increaseRef()
{
    ++mRef;
}

void Texture2D::decreaseRef()
{
    --mRef;
}

int Texture2D::getRef() const
{
    return mRef;
}

void Texture2D::initMaxTextureSize()
{
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
}

GLint Texture2D::getMaxTextureSize()
{
    return maxTextureSize;
}
