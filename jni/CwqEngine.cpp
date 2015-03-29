#include "CwqEngine.h"
#include <GLES2/gl2.h>
#include "base/CWQMacros.h"
#include "GraphicsService.h"
#include "Resource.h"
#include "LogHelper.h"

CwqEngine::CwqEngine()
{
    graphicsService = new GraphicsService();
}

CwqEngine::~CwqEngine()
{
    SAFE_DELETE(graphicsService);
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
    graphicsService->start();
    graphicsService->registerSprite("test.png");
    LOGE("onSurfaceCreated 2");
}

void CwqEngine::onSurfaceChanged(int width, int height)
{
    LOGE("onSurfaceChanged 1");
    graphicsService->screenSizeChanged(width, height);
    LOGE("onSurfaceChanged 2");
}

void CwqEngine::onDrawFrame()
{
    graphicsService->update(0);
}

void CwqEngine::onResume()
{
    LOGE("onResume");
}

void CwqEngine::onPause()
{
    LOGE("onPause");
    graphicsService->stop();
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
