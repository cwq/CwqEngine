#include "Resource.h"

AAssetManager* Resource::mAssetManager = NULL;

void Resource::setAssetManager(void* assetManager)
{
    mAssetManager = (AAssetManager*)assetManager;
}

Resource::Resource(const char* pPath)
: mPath(pPath), mAsset(NULL), fp(NULL)
{
    
}

bool Resource::open()
{
    bool ret = false;
    if(mPath[0] != '/')
    {
        //assets
        mAsset = AAssetManager_open(mAssetManager, mPath, AASSET_MODE_UNKNOWN);
        ret = mAsset != NULL;
    }
    else
    {
        //sdcard
        fp = fopen(mPath, "r");
        ret = fp != NULL;
    }
    return ret;
}

void Resource::close()
{
    if (mAsset != NULL)
    {
        AAsset_close(mAsset);
        mAsset = NULL;
    }
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
}

bool Resource::read(void* pBuffer, size_t pCount)
{
    size_t lReadCount = 0;
    if(mPath[0] != '/')
    {
        //assets
        lReadCount = AAsset_read(mAsset, pBuffer, pCount);
    }
    else
    {
        //sdcard
        lReadCount = fread(pBuffer, sizeof(unsigned char), pCount, fp);
    }
    return lReadCount == pCount;
}

const char* Resource::getPath()
{
    return mPath;
}

size_t Resource::getLength()
{
    size_t length = 0;
    if(mPath[0] != '/')
    {
        //assets
        length = AAsset_getLength(mAsset);
    }
    else
    {
        //sdcard
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }
    return length;
}
