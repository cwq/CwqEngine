#include "CwqEngine.h"
#include <GLES2/gl2.h>

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
}

void CwqEngine::onSurfaceCreated()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CwqEngine::onSurfaceChanged(int width, int height)
{
    glViewport(0, 0, width, height);
}

void CwqEngine::onDrawFrame()
{
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CwqEngine::onResume()
{

}

void CwqEngine::onPause()
{

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
