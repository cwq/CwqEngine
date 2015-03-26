#include "GraphicsService.h"
#include "base/CWQMacros.h"
#include "LogHelper.h"
#include "Texture2D.h"
#include "GraphicsSprite.h"
#include "renderer/Shader.h"
#include "renderer/TextureShader.h"
#include "renderer/BasicShader.h"
#include "CWQEngine.h"
//#include "ApplicationAdapter.h"

#include <cassert>

GraphicsService::GraphicsService()
{
  
}

GraphicsService::~GraphicsService() 
{
  
}

const int32_t& GraphicsService::getHeight() 
{
  return mHeight;
}

const int32_t& GraphicsService::getWidth() 
{
  return mWidth;
}


void GraphicsService::start()
{
    //Log::info("Starting GraphicsService");
    ////Log::info("Version  : %s", glGetString(GL_VERSION));
    //Log::info("Vendor   : %s", glGetString(GL_VENDOR));
    //Log::info("Renderer : %s", glGetString(GL_RENDERER));
    //Log::info("Viewport : %d x %d", mWidth, mHeight);

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

    setupVBO();

    mCommonShader = new TextureShader();
    registerShader(mCommonShader);
}

void GraphicsService::stop()
{
    //Log::info("Stopping GraphicsService.");
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

void GraphicsService::screenSizeChanged(int32_t &width, int32_t &height)
{
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

    Mat4::createOrthographicOffCenter(0, mWidth, 0, mHeight, -1, 1, &pMat);
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
}

void GraphicsService::fillQuads(GraphicsSprite *&pSprite, int playedTime)
{
//    const Mat4& modelView = cmd->getModelView();
    pSprite->update(playedTime);

    if (pSprite->isVisible()) {
        const V3F_C4F_T2F* quads =  (V3F_C4F_T2F*)pSprite->getQuads();
        for(ssize_t i=0; i< 4; ++i)
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(_quadVerts[0]) * VBO_SIZE, _quadVerts, GL_DYNAMIC_DRAW);

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

    {
#define kQuadSize sizeof(_quadVerts[0])
        glBindBuffer(GL_ARRAY_BUFFER, _quadbuffersVBO[0]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(_quadVerts[0]) * _numberQuads * 4 , _quadVerts, GL_DYNAMIC_DRAW);

//        GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
        glEnableVertexAttribArray(mCommonShader->m_positionAttributeHandle);
        glEnableVertexAttribArray(mCommonShader->m_texCoordAttributeHandle);
        ;
        // vertices
        glVertexAttribPointer(mCommonShader->m_positionAttributeHandle, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(V3F_C4F_T2F, vertices));

        // colors
//        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof(V3F_C4F_T2F, colors));

        // tex coords
        glVertexAttribPointer(mCommonShader->m_texCoordAttributeHandle, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(V3F_C4F_T2F, texCoords));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadbuffersVBO[1]);
    }

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

                mCommonShader->Setup(*pSprite);
                glUniformMatrix4fv(mCommonShader->m_matrixHandle, 1, GL_FALSE, mvpMat.m);
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
    pShader->Link();
    mShaders.push_back(pShader);
}
