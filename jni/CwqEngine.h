#ifndef CWQENGINE_H
#define CWQENGINE_H
#include <android/asset_manager.h>

class CwqEngine
{
public:
    CwqEngine();
    ~CwqEngine();
    void setAssetManager(AAssetManager* assetManager);
    
    void onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
    
    void onResume();
    void onPause();
    
    void onKeyDown(int keyCode);
    void onTouchesBegin(int pID, float pX, float pY);
    void onTouchesEnd(int pID, float pX, float pY);
    void onTouchesMove(int* pIDs, float* pXs, float* pYs, int pNum);
    void onTouchesCancel(int* pIDs, float* pXs, float* pYs, int pNum);
    
private:
    AAssetManager* mAssetManager;
};

#endif // !CWQENGINE_H
