#include "GraphicsService.h"
#include "base/CWQMacros.h"
#include "base/LogHelper.h"
#include "Texture2D.h"
#include "GraphicsSprite.h"
#include "Shader.h"
#include "TextureShader.h"
#include "TextureCache.h"

#include <cassert>

int GraphicsService::cacheNum = 0;

GraphicsService::GraphicsService()
{
    mWidth = mHeight = 0;
    mCommonShader = NULL;
    _numberQuads = 0;
    cacheIndex = 0;
    started = false;
}

GraphicsService::~GraphicsService() 
{
    SAFE_DELETE(mCommonShader);

    for (SpriteVectorIterator iter = mSprites.begin(); iter != mSprites.end(); ++iter)
    {
        GraphicsSprite* pCurrent = *iter;
        //SAFE_RELEASE(pCurrent);
        SAFE_DELETE(pCurrent);
    }
    mSprites.clear();
}

int GraphicsService::getHeight() const
{
  return mHeight;
}

int GraphicsService::getWidth() const
{
  return mWidth;
}


bool GraphicsService::start()
{
    //Log::info("Starting GraphicsService");
    ////Log::info("Version  : %s", glGetString(GL_VERSION));
    //Log::info("Vendor   : %s", glGetString(GL_VENDOR));
    //Log::info("Renderer : %s", glGetString(GL_RENDERER));
    //Log::info("Viewport : %d x %d", mWidth, mHeight);

    if (!started)
    {
        cacheIndex = cacheNum;
        ++cacheNum;
        if(cacheNum > TextureCache::MAX_CACHE)
        {
            LOGE("TextureCache maxCache is %i", TextureCache::MAX_CACHE);
            return false;
        }

        Mat4::createLookAt(Vec3(0, 0, 1), Vec3(0, 0, 0), Vec3(0, 1, 0), &vMat);

        //setup index data for quads
        for( int i=0; i < VBO_SIZE/4; i++)
        {
            _quadIndices[i*6+0] = (GLushort) (i*4+0);
            _quadIndices[i*6+1] = (GLushort) (i*4+1);
            _quadIndices[i*6+2] = (GLushort) (i*4+2);
            _quadIndices[i*6+3] = (GLushort) (i*4+3);
            _quadIndices[i*6+4] = (GLushort) (i*4+2);
            _quadIndices[i*6+5] = (GLushort) (i*4+1);
        }

        mCommonShader = new TextureShader();

        //bind current GraphicsService to TextureCache
        TextureCache::setCurrentCache(cacheIndex);

        started = true;
    }
    else
    {
        //bind current GraphicsService to TextureCache
        TextureCache::setCurrentCache(cacheIndex);
        //when destory by system, restart service shoult reload all textures
        TextureCache::reloadAllTextures();
    }

    //if started, should reload all gl resources
    setupVBO();

    registerShader(mCommonShader);

    return true;
}

void GraphicsService::end()
{
    //Log::info("Ending GraphicsService.");

    --cacheNum;

    //release all texture (video texture is not in textureCache)
    TextureCache::removeAllTextures(cacheIndex);
}

void GraphicsService::update(int playedTime)
{
    // Update graphics coordinates from world coordinates.
    static auto grey = 1.0f;
//
//    grey += 0.01f;
//
//    if (grey > 1.0f) {
//        grey = 0.0f;
//    }
//
    glClearColor(grey, grey, grey, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (mSprites.size() > 0) {
        for (auto sprite : mSprites)
        {
            if (sprite) {
                fillQuads(sprite, playedTime);
            }
        }
        drawBatchedQuads();
    }
}

void GraphicsService::screenSizeChanged(int width, int height)
{
    //bind current GraphicsService to TextureCache
    TextureCache::setCurrentCache(cacheIndex);

    if (!width || !height) {
        //Log::error("screenSizeChanged with invalid width %d, height %d", width, height);
        return;
    }
    if ((width == mWidth && height == mHeight)) {
        // Nothing change
        return;
    }

    mWidth  = width;
    mHeight = height;

    glViewport(0, 0, mWidth, mHeight);

    Mat4::createOrthographicOffCenter(0.0f, (float)mWidth, 0.0f, (float)mHeight, -1.0f, 1.0f, &pMat);
    Mat4::multiply(vMat, pMat, &mvpMat);
}

GraphicsSprite* GraphicsService::registerSprite(const std::string &filename) {
    GraphicsSprite *sprite = nullptr;

    sprite = GraphicsSprite::create(filename);

    if (sprite) {
        addSprite(sprite);
    }

    return sprite;
}

void GraphicsService::addSprite(GraphicsSprite *pSprite) {
    assert(pSprite);

    if (pSprite) {
        pSprite->setShader(mCommonShader);
        mSprites.push_back(pSprite);
    }
}

void GraphicsService::removeSprite(GraphicsSprite* pSprite)
{
    assert(pSprite);
    pSprite->release();
    for (SpriteVectorIterator iter = mSprites.begin(); iter != mSprites.end(); ++iter)
    {
        GraphicsSprite* pCurrent = *iter;
        if (pCurrent == pSprite)
        {
            mSprites.erase(iter);
            break;
        }
    }
    SAFE_DELETE(pSprite);
}

void GraphicsService::fillQuads(GraphicsSprite* pSprite, int playedTime)
{
//    const Mat4& modelView = cmd->getModelView();
    pSprite->update(playedTime);

    if (pSprite->isVisible()) {
        const V3F_C4F_T2F* quads =  (V3F_C4F_T2F*)pSprite->getQuads();
        for(int i=0; i< 4; ++i)
        {
            _quadVerts[i + _numberQuads * 4] = quads[i];
    //        modelView.transformPoint(quads[i].vertices,&(_quadVerts[i + _numberQuads * 4].vertices));
        }

        _numberQuads += 1;
    }
}

void GraphicsService::setupVBO()
{
    glGenBuffers(2, &_quadbuffersVBO[0]);
    mapBuffers();
}

void GraphicsService::mapBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, _quadbuffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, V3F_C4F_T2F_SIZE * VBO_SIZE, _quadVerts, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadbuffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_quadIndices[0]) * INDEX_VBO_SIZE, _quadIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERROR_DEBUG();
}

void GraphicsService::drawBatchedQuads()
{
    int indexToDraw = 0;
    int startIndex = 0;

    //Upload buffer to VBO
    if(_numberQuads <= 0)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //bind V3F_C4F_T2F data, and index data
    glBindBuffer(GL_ARRAY_BUFFER, _quadbuffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, V3F_C4F_T2F_SIZE * _numberQuads * 4 , _quadVerts, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadbuffersVBO[1]);

    Texture2D *last_material = nullptr;
    //Start drawing verties in batch
    for(const auto& pSprite : mSprites)
    {
        if (pSprite->isVisible()) {
            if (last_material!= pSprite->getTexture()) {
                //Draw quads
                if(indexToDraw > 0)
                {
                    glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(_quadIndices[0])) );
                    startIndex += indexToDraw;
                    indexToDraw = 0;
                }

                mCommonShader->setup(*pSprite, mvpMat.m);
                last_material = pSprite->getTexture();
            }

            indexToDraw += 6;
        }
    }

    //Draw any remaining quad
    if(indexToDraw > 0)
    {
        glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(_quadIndices[0])) );
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisable(GL_BLEND);

    _numberQuads = 0;
}

void GraphicsService::registerShader(Shader* pShader) {
    assert(pShader);
    pShader->link();
    //mShaders.push_back(pShader);
}
