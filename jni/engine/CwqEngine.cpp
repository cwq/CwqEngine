#include "CwqEngine.h"
#include "platform/CwqGL.h"
#include "base/CWQMacros.h"
#include "renderer/GraphicsService.h"
#include "platform/Resource.h"
#include "base/LogHelper.h"
#include "mediaplayer/MediaPlayer.h"
#include "renderer/GraphicsSprite.h"
#include "renderer/Texture2D.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include "platform/android/CwqEngineJNIHelper.h"
#endif // !ANDROID !__ANDROID__

int CwqEngine::engineNum = 0;

extern void CwqEngineInit();

CwqEngine::CwqEngine()
{
    exited = false;

    if (engineNum == 0)
    {
        CwqEngineInit();
    }
    ++engineNum;

    graphicsService = new GraphicsService();

    mediaPlayer = new MediaPlayer();

    videoTextures.push_back(new Texture2D());
    videoTextures.push_back(new Texture2D());

    GraphicsSprite* videoSprite = new GraphicsSprite();
    videoSprite->setTexture(videoTextures[0]);
    sprites.push_back(videoSprite);
    videoSprite = new GraphicsSprite();
    videoSprite->setTexture(videoTextures[1]);
    sprites.push_back(videoSprite);

    images.push_back(new Image());
    images.push_back(new Image());

    selectedSprit = NULL;
}

CwqEngine::~CwqEngine()
{
    for(int i = 0; i < videoTextures.size(); ++i) {
        SAFE_DELETE(images[i]);
        SAFE_DELETE(videoTextures[i]);
    }
    images.clear();
    videoTextures.clear();

    SAFE_DELETE(graphicsService);
    SAFE_DELETE(mediaPlayer);

    sprites.clear();
}

void CwqEngine::onExit()
{
    LOGE("onExit");
    exited = true;
    --engineNum;
    graphicsService->end();
    mediaPlayer->end();
}

void CwqEngine::postEventToEngine(bool handleOnGLThread, int what, int arg1, int arg2, void* obj)
{
    LOGE("postEventToEngine:%d, %d, %d, %d", handleOnGLThread, what, arg1, arg2);
}

void CwqEngine::onSurfaceCreated()
{
    LOGE("onSurfaceCreated 1");
    if (engineNum == 1)
        Texture2D::initMaxTextureSize();

    graphicsService->start();
    GraphicsSprite* sprite = GraphicsSprite::create("test.png");
//    GraphicsSprite* sprite = GraphicsSprite::create("C:/Users/DELL/Desktop/test.png");
    sprite->moveTo(60, 60);
    sprite->setWidthAndHeight(120, 120);
    sprites.push_back(sprite);

    sprites[0]->moveTo(240, 240);
    sprites[0]->setWidthAndHeight(480, 480);
    sprites[1]->moveTo(240, 240);
    sprites[1]->setWidthAndHeight(240, 240);
    graphicsService->addSprite(sprites[0]);
    graphicsService->addSprite(sprites[1]);

    graphicsService->addSprite(sprite);

    mediaPlayer->addMvTrack("/mnt/sdcard/test.mp4", 0, 0, 0, false);
    mediaPlayer->addMvTrack("/mnt/sdcard/test2.mp4", 0, 0, 0, false);
//    mediaPlayer->addMvTrack("C:/Users/DELL/Desktop/test.mp4", 0, 0, 0, false);
//    mediaPlayer->addMvTrack("C:/Users/DELL/Desktop/test2.mp4", 0, 0, 0, false);
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
    for(int i = 0; i < images.size(); ++i) {
        Image* image = images[i];
        if(image->isUpdated()) {
            videoTextures[i]->load(*image);
            sprites[i]->setTexture(videoTextures[i]);
            image->setUpdated(false);
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
    float x = pX;
    float y = graphicsService->getHeight() - pY;
    LOGE("touch begin (%f,%f)", x, y);
    for(int i = sprites.size() - 1; i >= 0; --i)
    {
        if(sprites[i]->isInSprite(x, y))
        {
            selectedSprit = sprites[i];
            return;
        }
    }
}

void CwqEngine::onTouchesEnd(int pID, float pX, float pY)
{
    selectedSprit = NULL;
}

void CwqEngine::onTouchesMove(int* pIDs, float* pXs, float* pYs, int pNum)
{
    float x = pXs[0];
    float y = graphicsService->getHeight() - pYs[0];
    if(selectedSprit != NULL)
        selectedSprit->moveTo(x, y);
}

void CwqEngine::onTouchesCancel(int* pIDs, float* pXs, float* pYs, int pNum)
{

}

#if defined(ANDROID) || defined(__ANDROID__)
void CwqEngine::setJavaWeakEngine(void* jWeakEngine)
{
    mJWeakEngine = jWeakEngine;
}

void* CwqEngine::getJavaWeakEngine()
{
    return mJWeakEngine;
}

void CwqEngine::postEventToJava(int what, int arg1, int arg2, void* obj)
{
    CwqEngineJNIHelper::postEventToJava((jobject)mJWeakEngine, what, arg1, arg2, (jobject)obj);
}
#endif // !ANDROID !__ANDROID__
