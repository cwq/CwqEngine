#include "Resource.h"

#if defined(ANDROID) || defined(__ANDROID__)

AAssetManager* Resource::mAssetManager = NULL;

void Resource::setAssetManager(AAssetManager* assetManager)
{
    mAssetManager = assetManager;
}

#endif // !ANDROID !__ANDROID__

Resource::Resource(const char* pPath)
: mPath(pPath), fp(NULL)
{
    #if defined(ANDROID) || defined(__ANDROID__)
    mAsset = NULL;
    #endif // !ANDROID !__ANDROID__
}

bool Resource::open()
{
    bool ret = false;

    #if defined(ANDROID) || defined(__ANDROID__)
    if(mPath[0] != '/')
    {
        //assets
        mAsset = AAssetManager_open(mAssetManager, mPath, AASSET_MODE_UNKNOWN);
        ret = mAsset != NULL;
        return ret;
    }
    #endif // !ANDROID !__ANDROID__

    //android sdcard or other platforms
    fp = fopen(mPath, "rb");
    ret = fp != NULL;
    return ret;
}

void Resource::close()
{
    #if defined(ANDROID) || defined(__ANDROID__)
    if (mAsset != NULL)
    {
        AAsset_close(mAsset);
        mAsset = NULL;
    }
    #endif // !ANDROID !__ANDROID__
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
}

bool Resource::read(void* pBuffer, size_t pCount) const
{
    size_t lReadCount = 0;

    #if defined(ANDROID) || defined(__ANDROID__)
    if(mPath[0] != '/')
    {
        //assets
        lReadCount = AAsset_read(mAsset, pBuffer, pCount);
        return lReadCount == pCount;
    }
    #endif // !ANDROID !__ANDROID__

    //android sdcard or other platforms
    lReadCount = fread(pBuffer, sizeof(unsigned char), pCount, fp);
    return lReadCount == pCount;
}

const char* Resource::getPath() const
{
    return mPath;
}

size_t Resource::getLength() const
{
    size_t length = 0;
    #if defined(ANDROID) || defined(__ANDROID__)
    if(mPath[0] != '/')
    {
        //assets
        length = AAsset_getLength(mAsset);
        return length;
    }
    #endif // !ANDROID !__ANDROID__

    //android sdcard or other platforms
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return length;
}
