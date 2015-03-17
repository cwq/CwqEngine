#include "GraphicsSprite.h"

#include <math.h>

#include "base/CWQMacros.h"
#include "Image.h"
#include "Texture2D.h"
#include "TextureCache.h"

GraphicsSprite::GraphicsSprite()
{
    setUV(0, 1, 0, 1);
}

GraphicsSprite::~GraphicsSprite()
{

}

GraphicsSprite* GraphicsSprite::create(const char* filename, int frameCount) {
    return create(std::string(filename), frameCount);
}

GraphicsSprite* GraphicsSprite::create(const std::string &filename, int frameCount)
{
    GraphicsSprite *sprite = nullptr;

    Texture2D *texture = TextureCache::GetSingletonPtr()->addTexture(filename);
    if (texture) {
    } else {
        goto fail;
    }

    sprite = createWithTexture(texture, frameCount);

    if (sprite) {
        return sprite;
    }
fail:
    SAFE_DELETE(sprite);
    SAFE_DELETE(texture);

    return nullptr;
}

GraphicsSprite* GraphicsSprite::createWithTexture(Texture2D *texture, int frameCount)
{
    GraphicsSprite *sprite = new (std::nothrow) GraphicsSprite();

    if (sprite && sprite->initWithTexture(texture, frameCount)) {
        return sprite;
    }

    SAFE_DELETE(sprite);
    return nullptr;
}

bool GraphicsSprite::initWithTexture(Texture2D *texture, int frameCount)
{
    if (!texture) {
        // Create empty texture.
        // For now do nothing.
        return false;
    }

    setTexture(texture);

    mAnimFrameCount = frameCount;
    float count_1 = 1.0f / frameCount;
    origWidth = (float)mTexture->getWidth() * count_1;
    origHeight = (float)mTexture->getHeight();
    setWidthAndHeight(origWidth, origHeight);
    moveTo(mWidth / 2.0, mHeight / 2.0);
    setUV(0, count_1, 0, 1);
    update(0);

    return true;
}

void GraphicsSprite::setTexture(Texture2D *texture) {

    if (!texture) {
        // Create empty texture.
        // For now do nothing.
        return;
    }

    if (texture != mTexture) {
        // delete last
        SAFE_DELETE(mTexture);
        mTexture = texture;
    }
}

void GraphicsSprite::update(int currentTime)
{
    if((currentTime >= mAnimStartTime && currentTime < mAnimEndTime) ||
        (mAnimFrameCount == 1 && mAnimDuration == 0))
    {
        if(mAnimFrameCount > 1)
        {
            int frameIndex = (currentTime - mAnimStartTime) / mAnimDuration * mAnimFrameCount;
            float count_1 = 1.0f / mAnimFrameCount;
            float startU = frameIndex * count_1;
            setUV(startU, startU + count_1, 0, 1);
        }

        mVisible = true;

        if(needUpdatePosition)
        {
            updatePosition();
            needUpdatePosition = false;
        }
        if(needUpdateUV)
        {
            updateUV();
            needUpdateUV = false;
        }

    }
    else
    {
        mVisible = false;
    }
}

void GraphicsSprite::updatePosition()
{
    float halfW = mWidth / 2.0;
    float halfH = mHeight / 2.0;

    double sinAngle = sin(rotateAngle);
    double cosAngle = cos(rotateAngle);
    double halfWCos = halfW * cosAngle;
    double halfHSin = halfH * sinAngle;
    double halfWSin = halfW * sinAngle;
    double halfHCos = halfH * cosAngle;

    mQuads.tl.vertices[0] = mCenterX - halfWCos - halfHSin;
    mQuads.tl.vertices[1] = mCenterY + halfHCos - halfWSin;
    mQuads.tl.vertices[2] = 0.0f;

    mQuads.bl.vertices[0] = mCenterX - halfWCos + halfHSin;
    mQuads.bl.vertices[1] = mCenterY - halfHCos - halfWSin;
    mQuads.bl.vertices[2] = 0.0f;

    mQuads.tr.vertices[0] = mCenterX + halfWCos - halfHSin;
    mQuads.tr.vertices[1] = mCenterY + halfHCos + halfWSin;
    mQuads.tr.vertices[2] = 0.0f;

    mQuads.br.vertices[0] = mCenterX + halfWCos + halfHSin;
    mQuads.br.vertices[1] = mCenterY - halfHCos + halfWSin;
    mQuads.br.vertices[2] = 0.0f;
}

void GraphicsSprite::updateUV()
{
    mQuads.tl.texCoords[0] = startU;//startU;
    mQuads.tl.texCoords[1] = 1 - endV;//endV;

    mQuads.bl.texCoords[0] = startU;//startU;
    mQuads.bl.texCoords[1] = 1 - startV;//startV;

    mQuads.tr.texCoords[0] = endU;//endU;
    mQuads.tr.texCoords[1] = 1 - endV;//endV;

    mQuads.br.texCoords[0] = endU;//endU;
    mQuads.br.texCoords[1] = 1 - startV;//startV;
}

Texture2D* GraphicsSprite::getTexture() const
{
    return mTexture;
}

void GraphicsSprite::setShader(Shader* pShader)
{
    mShader = pShader;
}

Shader* GraphicsSprite::getShader()
{
    return mShader;
}

void GraphicsSprite::setWidthAndHeight(float width, float height)
{
    if(mWidth != width || mHeight != height)
    {
        mWidth = width;
        mHeight = height;
        needUpdatePosition = true;
    }
}

void GraphicsSprite::moveTo(float cX, float cY)
{
    if(mCenterX != cX || mCenterY != cY)
    {
        mCenterX = cX;
        mCenterY = cY;
        needUpdatePosition = true;
    }
}

void GraphicsSprite::moveBy(float dX, float dY)
{
    mCenterX += dX;
    mCenterY += dY;
    needUpdatePosition = true;
}

void GraphicsSprite::setUV(float sU, float eU, float sV, float eV)
{
    if(startU != sU || endU != eU || startV != sV || endV != eV)
    {
        startU = sU;
        endU = eU;
        startV = sV;
        endV = eV;
        needUpdateUV = true;
    }
}

void GraphicsSprite::rotateTo(float angle)
{
    if(rotateAngle != angle)
    {
        rotateAngle = angle;
        needUpdatePosition = true;
    }
}

void GraphicsSprite::rotateBy(float angle)
{
    rotateAngle += angle;
    needUpdatePosition = true;
}

void GraphicsSprite::scaleTo(float scale)
{
    setWidthAndHeight(origWidth*scale, origHeight*scale);
}

void GraphicsSprite::scaleBy(float scale)
{
    setWidthAndHeight(mWidth*scale, mHeight*scale);
}

bool GraphicsSprite::isInSprite(float pointX, float pointY)
{
    float tempX = pointX - mCenterX;
    float tempY = pointY - mCenterY;
    double sinAngle = sin(rotateAngle);
    double cosAngle = cos(rotateAngle);
    pointX = cosAngle * tempX + sinAngle * tempY;
    pointY = -sinAngle * tempX + cosAngle * tempY;

    float halfW = mWidth / 2.0;
    float halfH = mHeight / 2.0;
    if(pointX >= -halfW && pointX <= halfW && pointY >= -halfH && pointY <= halfH)
        return true;

    return false;
}

void GraphicsSprite::setAnimStartTime(int startTime)
{
    mAnimStartTime = startTime;
    mAnimDuration = mAnimEndTime - mAnimStartTime;
}

void GraphicsSprite::setAnimEndTime(int endTime)
{
    mAnimEndTime = endTime;
    mAnimDuration = mAnimEndTime - mAnimStartTime;
}

void GraphicsSprite::release()
{
    //TextureCache release
    TextureCache::GetSingletonPtr()->releaseTexture(mTexture);
    mTexture = nullptr;
}
