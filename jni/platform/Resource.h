#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdio.h>
#include <android/asset_manager.h>

class Resource
{
public:
    Resource(const char* pPath);

    const char* getPath() const;

    bool open();
    void close();
    bool read(void* pBuffer, size_t pCount) const;

    size_t getLength() const;

    static void setAssetManager(void* assetManager);

protected:
    const char* mPath;

    FILE *fp;

    static AAssetManager* mAssetManager;
    AAsset* mAsset;
};

#endif // !RESOURCE_H
