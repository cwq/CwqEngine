#include "../ijksdl_aout.h"
#include "mediaplayer/ijkutil/ijkutil.h"
#include "mediaplayer/ijkutil/ijksdl_thread.h"

SDL_Aout *SDL_AoutCreate()
{
    SDL_Aout *aout = (SDL_Aout*) mallocz(sizeof(SDL_Aout));
    if (!aout)
        return NULL;

    aout->mutex = SDL_CreateMutex();
    if (aout->mutex == NULL)
    {
        free(aout);
        return NULL;
    }

    return aout;
}
