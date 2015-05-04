#include "CwqEngine.h"
#include "platform/CwqGL.h"
#include "base/CWQMacros.h"
#include "renderer/GraphicsService.h"
#include "platform/Resource.h"
#include "base/LogHelper.h"
#include "mediaplayer/MediaPlayer.h"
#include "renderer/GraphicsSprite.h"
#include "renderer/Texture2D.h"

CwqEngine::CwqEngine()
{
    exited = false;

    mJWeakEngine = NULL;

    graphicsService = new GraphicsService();

    mediaPlayer = new MediaPlayer();
    videoImage = new Image();
    videoTexture = new Texture2D();
    videoSprite = new GraphicsSprite();
    videoSprite->setTexture(videoTexture);

    images.push_back(videoImage);
}

CwqEngine::~CwqEngine()
{
    images.clear();
    SAFE_DELETE(graphicsService);
    SAFE_DELETE(mediaPlayer);
    SAFE_DELETE(videoImage);
    SAFE_DELETE(videoTexture);
}

void CwqEngine::onExit()
{
    LOGE("onExit");
    exited = true;
    graphicsService->end();
    mediaPlayer->end();
}

void CwqEngine::setJavaWeakEngine(void* jWeakEngine)
{
    mJWeakEngine = jWeakEngine;
}

void* CwqEngine::getJavaWeakEngine()
{
    return mJWeakEngine;
}

void CwqEngine::onSurfaceCreated()
{
    LOGE("onSurfaceCreated 1");
    graphicsService->start();
    graphicsService->registerSprite("test.png");

    graphicsService->addSprite(videoSprite);
    mediaPlayer->addMvTrack("/mnt/sdcard/test.mp4", 0, 0, 0, false);
    mediaPlayer->start();
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
    if(exited)
        return;

    int remaingTimes = 0;
    mediaPlayer->getNextFrame(&remaingTimes, images);
    for(Image* image : images) {
        videoTexture->load(*image);
        videoSprite->setTexture(videoTexture);
        videoSprite->moveTo(500, 500);
        videoSprite->setWidthAndHeight(480, 480);
    }

    graphicsService->update(0);
}

void CwqEngine::onResume()
{
    LOGE("onResume");
    mediaPlayer->resume();
}

void CwqEngine::onPause()
{
    LOGE("onPause");
    if(exited)
        return;

    mediaPlayer->pause();
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
