#ifndef CWQENGINE_H
#define CWQENGINE_H

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

    void setAssetManager(void* assetManager);

    void onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
    
    void onResume();
    void onPause();
    void onDestroy();
    
    void onKeyDown(int keyCode);
    void onTouchesBegin(int pID, float pX, float pY);
    void onTouchesEnd(int pID, float pX, float pY);
    void onTouchesMove(int* pIDs, float* pXs, float* pYs, int pNum);
    void onTouchesCancel(int* pIDs, float* pXs, float* pYs, int pNum);

private:
    void* mJWeakEngine;
    void* mAssetManager;

    GraphicsService* graphicsService;

    MediaPlayer* mediaPlayer;
    GraphicsSprite* videoSprite;
    Texture2D* videoTexture;
    Image* videoImage;
};

#endif // !CWQENGINE_H
