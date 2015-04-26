#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

//#include "base/Singleton.h"
#include <string>
#include <unordered_map>

class Texture2D;

class TextureCache// : public Singleton<TextureCache>
{
public:
//    TextureCache();
//    ~TextureCache();

    static const int MAX_CACHE = 3;

    static void setCurrentCache(int index);

    static Texture2D* addTexture(const std::string &fileName);

    static void releaseTexture(const std::string &fileName);
    static void releaseTexture(Texture2D* texture);

    static bool reloadTexture(const std::string& fileName);
    static bool reloadTexture(Texture2D* texture);
    static void reloadAllTextures();

    static void removeAllTextures(int cacheIndex);

private:
    static std::unordered_map<std::string, Texture2D*> _textures[MAX_CACHE];
    static int currentIndex;
};

#endif // !TEXTURECACHE_H
