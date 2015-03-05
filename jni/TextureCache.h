#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

#include "Singleton.h"
#include <string>
#include <unordered_map>

class Texture2D;

class TextureCache : public Singleton<TextureCache>
{
public:
    TextureCache();
    ~TextureCache();

    Texture2D* addTexture(const std::string &fileName);

    void releaseTexture(const std::string &fileName);
    void releaseTexture(Texture2D* texture);

    bool reloadTexture(const std::string& fileName);
    bool reloadTexture(Texture2D* texture);
    void reloadAllTexture();

    void removeAllTextures();

private:
    std::unordered_map<std::string, Texture2D*> _textures;
};

#endif // !TEXTURECACHE_H
