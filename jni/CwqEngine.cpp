#include "CwqEngine.h"
#include <GLES2/gl2.h>
#include "Texture2D.h"
#include "Resource.h"
#include "LogHelper.h"

extern bool setupGraphics();
extern void surfaceChanged(int w, int h);
extern void renderFrame(int textureID);
Texture2D texture;

CwqEngine::CwqEngine()
{

}

CwqEngine::~CwqEngine()
{

}

void CwqEngine::setJavaWeakEngine(void* jWeakEngine)
{
    mJWeakEngine = jWeakEngine;
}

void* CwqEngine::getJavaWeakEngine()
{
    return mJWeakEngine;
}

void CwqEngine::setAssetManager(void* assetManager)
{
    //AAssetManager
    mAssetManager = assetManager;
    //static Resource::setAssetManager
    Resource::setAssetManager(mAssetManager);
}

void CwqEngine::onSurfaceCreated()
{
    LOGE("onSurfaceCreated 1");
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    setupGraphics();
    //texture.load("/mnt/sdcard/test.png");
    texture.load("test.png");
    LOGE("onSurfaceCreated 2");
}

void CwqEngine::onSurfaceChanged(int width, int height)
{
    LOGE("onSurfaceChanged 1");
    surfaceChanged(width, height);
    LOGE("onSurfaceChanged 2");
}

void CwqEngine::onDrawFrame()
{
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    renderFrame(texture.getTextureID());
}

void CwqEngine::onResume()
{
    LOGE("onResume");
}

void CwqEngine::onPause()
{
    LOGE("onPause");
}

void CwqEngine::onKeyDown(int keyCode)
{

}

void CwqEngine::onTouchesBegin(int pID, float pX, float pY)
{

}

void CwqEngine::onTouchesEnd(int pID, float pX, float pY)
{

}

void CwqEngine::onTouchesMove(int* pIDs, float* pXs, float* pYs, int pNum)
{

}

void CwqEngine::onTouchesCancel(int* pIDs, float* pXs, float* pYs, int pNum)
{

}
