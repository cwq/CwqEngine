#include "TextureCache.h"
#include "base/CWQMacros.h"
#include "Texture2D.h"

TextureCache::TextureCache()
{

}

TextureCache::~TextureCache()
{

}

Texture2D* TextureCache::addTexture(const std::string &fileName)
{
    Texture2D * texture = nullptr;
    auto it = _textures.find(fileName);
    if( it != _textures.end() )
    {
        texture = it->second;
        texture->increaseRef();
    }
    else
    {
        texture = new Texture2D();
        if (texture && texture->load(fileName.c_str()))
        {
            _textures.insert( std::make_pair(fileName, texture) );
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

    auto it = _textures.find(fileName);
    if (it != _textures.end()) {
        texture = it->second;
    }

    releaseTexture(texture);
}

void TextureCache::releaseTexture(Texture2D* texture)
{
    if(texture)
    {
        texture->decreaseRef();
        if(texture->getRef() <= 0)
        {
            texture->unLoad();
            for( auto it=_textures.cbegin(); it!=_textures.cend();)
            {
                if( it->second == texture )
                {
                    _textures.erase(it++);
                    break;
                } else
                    ++it;
            }
        }
    }
}

bool TextureCache::reloadTexture(const std::string& fileName)
{
    Texture2D * texture = nullptr;

    auto it = _textures.find(fileName);
    if (it != _textures.end()) {
        texture = it->second;
    }

    bool ret = false;
    if (! texture) {
        texture = this->addTexture(fileName);
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
        for( auto it=_textures.cbegin(); it!=_textures.cend();)
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

void TextureCache::reloadAllTexture()
{
    for( auto it=_textures.begin(); it!=_textures.end(); ++it ) {
        Image image;
        bool bRet = image.initWithFileName(it->first.c_str());
        if(bRet)
        {
            it->second->load(image);
        }
    }
}

void TextureCache::removeAllTextures()
{
    for( auto it=_textures.begin(); it!=_textures.end(); ++it ) {
        (it->second)->unLoad();
    }
    _textures.clear();
}
