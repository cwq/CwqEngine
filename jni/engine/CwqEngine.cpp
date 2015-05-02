#include "CwqEngine.h"
#include "platform/CwqGL.h"
#include "base/CWQMacros.h"
#include "renderer/GraphicsService.h"
#include "platform/Resource.h"
#include "base/LogHelper.h"
#include "mediaplayer/MediaPlayer.h"
#include "renderer/GraphicsSprite.h"
#include "renderer/Texture2D.h"

static void copyFromAVFrame(u_char *pixels, AVFrame *frame, int width, int height) {
    if (!frame || !pixels) {
        return;
    }

    int y = 0;
    int numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);

    for (y = 0; y < height; y++) {
        memcpy(pixels + (y * width * 3), frame->data[0] + y * frame->linesize[0], width * 3);
    }
}

CwqEngine::CwqEngine()
{
    exited = false;

    mJWeakEngine = NULL;
    mAssetManager = NULL;

    graphicsService = new GraphicsService();

    mediaPlayer = new MediaPlayer();
    videoImage = new Image();
    videoTexture = new Texture2D();
    videoSprite = new GraphicsSprite();
    videoSprite->setTexture(videoTexture);
}

CwqEngine::~CwqEngine()
{
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
    vector<VideoPicture *> pictures = mediaPlayer->getNextFrame(&remaingTimes);
    if(pictures.size() > 0) {
        VideoPicture* vp = pictures[0];
        if(vp != NULL) {
            videoImage->initWithImageInfo(vp->width, vp->height, GL_RGB);
            copyFromAVFrame((u_char*)videoImage->getPixels(), vp->decodedFrame, vp->width, vp->height);
            videoTexture->load(*videoImage);
            videoSprite->setTexture(videoTexture);
            videoSprite->moveTo(500, 500);
            videoSprite->setWidthAndHeight(480, 480);
        }
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
