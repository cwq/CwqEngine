#ifndef CWQENGINE_H
#define CWQENGINE_H

#include <vector>
class GraphicsService;
class MediaPlayer;
class Image;
class Texture2D;
class GraphicsSprite;

class CwqEngine
{
public:
    CwqEngine();
    ~CwqEngine();

    void postEventToEngine(bool handleOnGLThread, int what, int arg1, int arg2, void* obj);

    void onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
    
    void onResume();
    void onPause();
    void onExit();
    
    void onKeyDown(int keyCode);
    void onTouchesBegin(int pID, float pX, float pY);
    void onTouchesEnd(int pID, float pX, float pY);
    void onTouchesMove(int* pIDs, float* pXs, float* pYs, int pNum);
    void onTouchesCancel(int* pIDs, float* pXs, float* pYs, int pNum);

    #if defined(ANDROID) || defined(__ANDROID__)
    void setJavaWeakEngine(void* jWeakEngine);
    void* getJavaWeakEngine();
    void postEventToJava(int what, int arg1, int arg2, void* obj);
    #endif // !ANDROID !__ANDROID__

private:
    static int engineNum;

    bool exited;

    #if defined(ANDROID) || defined(__ANDROID__)
    void* mJWeakEngine;
    #endif // !ANDROID !__ANDROID__

    GraphicsService* graphicsService;

    MediaPlayer* mediaPlayer;
    GraphicsSprite* videoSprite;
    Texture2D* videoTexture;
    Image* videoImage;

    std::vector<Image *> images;
};

#endif // !CWQENGINE_H
