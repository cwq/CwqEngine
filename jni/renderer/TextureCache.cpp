#include "TextureCache.h"
#include "base/CWQMacros.h"
#include "Texture2D.h"
#include "base/LogHelper.h"

std::unordered_map<std::string, Texture2D*> TextureCache::_textures[MAX_CACHE];
int TextureCache::currentIndex = 0;

//TextureCache::TextureCache()
//{
//
//}
//
//TextureCache::~TextureCache()
//{
//
//}

void TextureCache::setCurrentCache(int index)
{
    if(index >=0 && index < MAX_CACHE)
    {
        currentIndex = index;
    }
    else
    {
        LOGE("TextureCache maxCache is %i, index is: %i", MAX_CACHE, index);
    }

}

Texture2D* TextureCache::addTexture(const std::string &fileName)
{
    Texture2D * texture = nullptr;
    auto it = _textures[currentIndex].find(fileName);
    if( it != _textures[currentIndex].end() )
    {
        texture = it->second;
        texture->increaseRef();
    }
    else
    {
        texture = new Texture2D();
        if (texture && texture->load(fileName.c_str()))
        {
            _textures[currentIndex].insert( std::make_pair(fileName, texture) );
        }
        else
        {
            SAFE_DELETE(texture);
        }
    }
    return texture;
}

void TextureCache::releaseTexture(const std::string &fileName)
{
    Texture2D * texture = nullptr;

    auto it = _textures[currentIndex].find(fileName);
    if (it != _textures[currentIndex].end()) {
        texture = it->second;
    }

    if(texture)
    {
        texture->decreaseRef();
        if(texture->getRef() <= 0)
        {
            texture->unLoad();
            _textures[currentIndex].erase(it);

            SAFE_DELETE(texture);
        }
    }
}

void TextureCache::releaseTexture(Texture2D* texture)
{
    if(texture)
    {
        texture->decreaseRef();
        if(texture->getRef() <= 0)
        {
            texture->unLoad();
            for( auto it=_textures[currentIndex].cbegin(); it!=_textures[currentIndex].cend();)
            {
                if( it->second == texture )
                {
                    _textures[currentIndex].erase(it);
                    break;
                } else
                    ++it;
            }

            SAFE_DELETE(texture);
        }
    }
}

bool TextureCache::reloadTexture(const std::string& fileName)
{
    Texture2D * texture = nullptr;

    auto it = _textures[currentIndex].find(fileName);
    if (it != _textures[currentIndex].end()) {
        texture = it->second;
    }

    bool ret = false;
    if (! texture) {
        texture = TextureCache::addTexture(fileName);
        ret = (texture != nullptr);
    }
    else
    {
        do {
            Image image;

            bool bRet = image.initWithFileName(fileName.c_str());
            BREAK_IF(!bRet);

            ret = texture->load(image);
        } while (0);
    }

    return ret;
}

bool TextureCache::reloadTexture(Texture2D* texture)
{
    bool ret = false;
    if(texture)
    {
        for( auto it=_textures[currentIndex].cbegin(); it!=_textures[currentIndex].cend();)
        {
            if( it->second == texture )
            {
                Image image;
                bool bRet = image.initWithFileName(it->first.c_str());
                BREAK_IF(!bRet);
                ret = texture->load(image);
                break;
            } else
                ++it;
        }
    }

    return ret;
}

void TextureCache::reloadAllTextures()
{
    for( auto it=_textures[currentIndex].begin(); it!=_textures[currentIndex].end(); ++it ) {
        Image image;
        bool bRet = image.initWithFileName(it->first.c_str());
        if(bRet)
        {
            it->second->load(image);
        }
    }
}

void TextureCache::removeAllTextures(int cacheIndex)
{
    for( auto it=_textures[cacheIndex].begin(); it!=_textures[cacheIndex].end(); ++it ) {
        Texture2D* texture = it->second;
        texture->unLoad();
        SAFE_DELETE(texture);
    }
    _textures[cacheIndex].clear();
}
