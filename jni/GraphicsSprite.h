#ifndef GRAPHICSSPRITE_H
#define GRAPHICSSPRITE_H

#include <string>
#include <GLES2/gl2.h>

#include "base/Types.h"

class Texture2D;
//class Shader;

class GraphicsSprite {
public:
    GraphicsSprite();
    virtual ~GraphicsSprite();

    static GraphicsSprite* create(const char* filename, int frameCount = 1);
    static GraphicsSprite* create(const std::string& filename, int frameCount = 1);
    static GraphicsSprite* createWithTexture(Texture2D* texture, int frameCount = 1);

    void setTexture(Texture2D* texture);
    Texture2D* getTexture() const;

    //void setShader(Shader* pShader);
    //Shader* getShader();

    float getWidth() const { return mWidth; };
    float getHeight() const { return mHeight; };
    void setWidthAndHeight(float width, float height);

    float getCenterX() const { return mCenterX; };
    float getCenterY() const { return mCenterY; };
    void moveTo(float cX, float cY);
    void moveBy(float dX, float dY);

    float getStartU() const { return startU; };
    float getEndU() const { return endU; };
    float getStartV() const { return startV; };
    float getEndV() const { return endV; };
    void setUV(float sU, float eU, float sV, float eV);

    float getRotateAngle() const { return rotateAngle; };
    void rotateTo(float angle);
    void rotateBy(float angle);

    void scaleTo(float scale);
    void scaleBy(float scale);

    bool isInSprite(float pointX, float pointY);

    bool isVisible() const { return mVisible; };
    void setVisible(bool visible) { mVisible = visible; };

    float getAnimStartTime() const { return mAnimStartTime; };
    void setAnimStartTime(float startTime);
    float getAnimEndTime() const { return mAnimEndTime; };
    void setAnimEndTime(float endTime);

    void update(float currentTime);

    void release();

    inline const V3F_C4F_T2F_Quad* getQuads() const { return &mQuads; };

protected:

    bool initWithTexture(Texture2D* texture, int frameCount = 1);

    void updatePosition();
    void updateUV();
private:
    // 纹理
    Texture2D*  mTexture    = nullptr;

    // 精灵中心坐标、宽高
    float mCenterX, mCenterY;
    float mWidth, mHeight;

    // 精灵原始宽高
    float origWidth, origHeight;

    // 使用纹理的UV坐标
    float startU, endU; //x
    float startV, endV; //y

    // 旋转角度 正数为逆时针
    float rotateAngle = 0;

    // 坐标几何数据存储
    V3F_C4F_T2F_Quad mQuads;

    // 数据是否需要更新
    bool needUpdatePosition = false;
    bool needUpdateUV = false;
    
    // 绘制使用的脚本
    //Shader*      mShader    = nullptr;

    // 是否可见
    bool mVisible = true;

    // Animation.
    int mAnimFrameCount = 1;
    float mAnimStartTime = 0, mAnimEndTime = 0;
    float mAnimDuration = 0;
};


#endif // !GRAPHICSSPRITE_H
