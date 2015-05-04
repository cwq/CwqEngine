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
    
    void setJavaWeakEngine(void* jWeakEngine);
    void* getJavaWeakEngine();

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

private:
    bool exited;

    void* mJWeakEngine;

    GraphicsService* graphicsService;

    MediaPlayer* mediaPlayer;
    GraphicsSprite* videoSprite;
    Texture2D* videoTexture;
    Image* videoImage;

    std::vector<Image *> images;
};

#endif // !CWQENGINE_H
