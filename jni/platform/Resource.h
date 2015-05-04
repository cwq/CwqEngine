#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdio.h>

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/asset_manager.h>
#endif // !ANDROID !__ANDROID__

class Resource
{
public:
    Resource(const char* pPath);

    const char* getPath() const;

    bool open();
    void close();
    bool read(void* pBuffer, size_t pCount) const;

    size_t getLength() const;

    #if defined(ANDROID) || defined(__ANDROID__)
    static void setAssetManager(AAssetManager* assetManager);
    #endif // !ANDROID !__ANDROID__

protected:
    const char* mPath;

    FILE *fp;

    #if defined(ANDROID) || defined(__ANDROID__)
    static AAssetManager* mAssetManager;
    AAsset* mAsset;
    #endif // !ANDROID !__ANDROID__
};

#endif // !RESOURCE_H
