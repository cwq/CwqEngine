#ifndef GRAPHICSSERVICE_H
#define GRAPHICSSERVICE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <GLES2/gl2.h>

#include "base/Types.h"
#include "math/Mat4.h"
#include "TextureCache.h"

class CWQEngine;
class GraphicsSprite;
class Shader;
class TextureShader;

class GraphicsService {
public:
    static const int VBO_SIZE = 65536;
    static const int INDEX_VBO_SIZE = VBO_SIZE * 6 / 4;

    typedef std::vector<GraphicsSprite*> SpriteVector;
    typedef std::vector<GraphicsSprite*>::iterator SpriteVectorIterator;
    GraphicsService(CWQEngine* engine);
    ~GraphicsService();
    
    const int32_t& getHeight();
    const int32_t& getWidth();

    void start();
    void stop();
    //ms
    void update(int playedTime);

    void screenSizeChanged(int32_t& width, int32_t& height);

    GraphicsSprite* registerSprite(const std::string& filename);

    void addSprite(GraphicsSprite* pSprite);
    void removeSprite(GraphicsSprite* pSprite);

    void registerShader(Shader* pShader);
    void removeShader(Shader* pShader);
protected:

    void setupVBO();
    void mapBuffers();
    
    void Draw(GraphicsSprite* pSprite);
    void drawBatchedQuads();

    void fillQuads(GraphicsSprite* &pSprite, int playedTime);

private:
    int32_t mWidth      = 0;
    int32_t mHeight     = 0;

    CWQEngine* mEngine   = nullptr;

    TextureShader* mCommonShader;
    // Graphics resources.
//    GraphicsSprite*     mSprites[256]   = {0};
    int32_t             mSpriteCount    = 0;

    TextureCache textureCache;

    SpriteVector mSprites;
    std::vector<Shader*> mShaders;

    //for QuadCommand
    //sprite info
    V3F_C4F_T2F _quadVerts[VBO_SIZE];
    GLushort _quadIndices[INDEX_VBO_SIZE];
    GLuint _quadVAO;
    GLuint _quadbuffersVBO[2]; //0: vertex  1: indices
    int _numberQuads;

    Mat4 vMat;
    Mat4 pMat;
    Mat4 mvpMat;

};

#endif // !GRAPHICSSERVICE_H
