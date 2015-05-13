#include "../ijksdl_aout.h"

#include <assert.h>
#include "mediaplayer/ijkutil/ijkutil.h"
#include "mediaplayer/ijkutil/ijksdl_thread.h"
#include <windows.h>

static const float MAX_VOLUME = 1;
static const float MIN_VOLUME = 0;
static const DWORD FULL_VOLUME = 0xFFFF;
static const int VOLUME_BIT = 16;

typedef struct SDL_Aout_Opaque {
    SDL_cond *wakeup_cond;
    SDL_mutex *wakeup_mutex;

    //要送到硬件的参数
    SDL_AudioSpec spec;
    //音轨参数
    //SDL_AndroidAudioTrack* atrack;

    //waveout
    HWAVEOUT hWaveOut; /* device handle */
    WAVEFORMATEX wfx;  /* formate */
    WAVEHDR wheader1;    /* wave data block header */
    WAVEHDR wheader2;    /* wave data block header */

    uint8_t *buffer;
    int buffer_size;

    volatile bool need_flush;
    volatile bool pause_on;
    volatile bool abort_request;

    volatile bool need_set_volume;
    volatile DWORD dwVolume;

    SDL_Thread *audio_tid;
    SDL_Thread _audio_tid;
} SDL_Aout_Opaque;

inline static SDL_Aout *SDL_Aout_CreateInternal(size_t opaque_size) {
    SDL_Aout *aout = (SDL_Aout*) mallocz(sizeof(SDL_Aout));
    if (!aout)
        return NULL;

    aout->opaque = (SDL_Aout_Opaque *) mallocz(opaque_size);
    if (!aout->opaque) {
        free(aout);
        return NULL;
    }

    aout->mutex = SDL_CreateMutex();
    if (aout->mutex == NULL) {
        free(aout->opaque);
        free(aout);
        return NULL;
    }

    return aout;
}

inline static void SDL_Aout_FreeInternal(SDL_Aout *aout) {
    if (!aout)
        return;

    if (aout->mutex) {
        SDL_DestroyMutex(aout->mutex);
    }

    free(aout->opaque);
    memset(aout, 0, sizeof(SDL_Aout));
    free(aout);
}

//waveout callback
void CALLBACK waveOutProc(HWAVEOUT hWave, UINT uMsg, DWORD dwInstance, DWORD dw1, DWORD dw2)
{
    SDL_Aout_Opaque *opaque = (SDL_Aout_Opaque *)dwInstance;
    SDL_AudioCallback audio_cblk = opaque->spec.callback;
    void *userdata = opaque->spec.userdata;
    switch (uMsg)  
    {  
    case WOM_DONE://last buff used
        LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
        if (!opaque->abort_request)
        {
            pWaveHeader->dwBufferLength = 0;
        }
        break;
    }  
}  

int aout_thread(void *arg) {
    SDL_Aout *aout = (SDL_Aout *)arg;
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_AudioCallback audio_cblk = opaque->spec.callback;
    void *userdata = opaque->spec.userdata;
    uint8_t *buffer = opaque->buffer;
    int copy_size = 4096;//2220 - 4100
    if (opaque->buffer_size < copy_size * 2)
    {
        copy_size = opaque->buffer_size / 2;
    }

    assert(buffer);

    WAVEHDR* wh1 = &(opaque->wheader1);
    WAVEHDR* wh2 = &(opaque->wheader2);

    wh1->dwBufferLength = copy_size;
    wh1->lpData = (LPSTR)buffer;
    wh2->dwBufferLength = copy_size;
    wh2->lpData = (LPSTR)(buffer + copy_size);

    audio_cblk(userdata, (Uint8 *)(wh1->lpData), wh1->dwBufferLength);
    waveOutPrepareHeader(opaque->hWaveOut, wh1, sizeof(WAVEHDR));
    waveOutWrite(opaque->hWaveOut, wh1, sizeof(WAVEHDR));
    audio_cblk(userdata, (Uint8 *)(wh2->lpData), wh2->dwBufferLength);
    waveOutPrepareHeader(opaque->hWaveOut, wh2, sizeof(WAVEHDR));
    waveOutWrite(opaque->hWaveOut, wh2, sizeof(WAVEHDR));

    waveOutSetVolume(opaque->hWaveOut, (FULL_VOLUME << VOLUME_BIT) + FULL_VOLUME);

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    if (!opaque->abort_request && !opaque->pause_on)
        waveOutRestart(opaque->hWaveOut);

    while (!opaque->abort_request) {
        SDL_LockMutex(opaque->wakeup_mutex);
        if (!opaque->abort_request && opaque->pause_on) {
            waveOutPause(opaque->hWaveOut);
            while (!opaque->abort_request && opaque->pause_on) {
                SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex,
                    1000);
            }
            if (!opaque->abort_request && !opaque->pause_on)
                waveOutRestart(opaque->hWaveOut);
        }
        if (opaque->need_flush) {
            opaque->need_flush = 0;
/*            sdl_audiotrack_flush(env, atrack);*/
        }
        if (opaque->need_set_volume) {
            opaque->need_set_volume = 0;
            waveOutSetVolume(opaque->hWaveOut, opaque->dwVolume);
        }
        SDL_UnlockMutex(opaque->wakeup_mutex);

        if (opaque->need_flush) {
/*            sdl_audiotrack_flush(env, atrack);*/
            opaque->need_flush = false;
        }

        if (opaque->need_flush) {
            opaque->need_flush = 0;
/*            sdl_audiotrack_flush(env, atrack);*/
        } else {
            if (wh1->dwBufferLength == 0)
            {
                wh1->dwBufferLength = copy_size;
                audio_cblk(userdata, (Uint8 *)(wh1->lpData), wh1->dwBufferLength);
                waveOutPrepareHeader(opaque->hWaveOut, wh1, sizeof(WAVEHDR));
                waveOutWrite(opaque->hWaveOut, wh1, sizeof(WAVEHDR));
            }
            if (wh2->dwBufferLength == 0)
            {
                wh2->dwBufferLength = copy_size;
                audio_cblk(userdata, (Uint8 *)(wh2->lpData), wh2->dwBufferLength);
                waveOutPrepareHeader(opaque->hWaveOut, wh2, sizeof(WAVEHDR));
                waveOutWrite(opaque->hWaveOut, wh2, sizeof(WAVEHDR));
            }
        }

        // TODO: 1 if callback return -1 or 0
    }

    waveOutReset(opaque->hWaveOut);

    waveOutUnprepareHeader(opaque->hWaveOut, wh1, sizeof(WAVEHDR));
    waveOutUnprepareHeader(opaque->hWaveOut, wh2, sizeof(WAVEHDR));

    waveOutClose(opaque->hWaveOut);
    return 0;
}

int aout_open_audio(SDL_Aout *aout, SDL_AudioSpec *desired,
                    SDL_AudioSpec *obtained) {
    assert(desired);
    SDL_Aout_Opaque *opaque = aout->opaque;

    opaque->spec = *desired;

    //waveout
    WAVEFORMATEX* waveformat = &(opaque->wfx);
    waveformat->wFormatTag = WAVE_FORMAT_PCM;
    waveformat->nChannels = desired->channels;
    waveformat->nSamplesPerSec = desired->freq;
    waveformat->wBitsPerSample = SDL_AUDIO_BITSIZE(desired->format);
    waveformat->nBlockAlign = waveformat->nChannels * (waveformat->wBitsPerSample / 8);
    waveformat->nAvgBytesPerSec = waveformat->nSamplesPerSec * waveformat->nBlockAlign;
    waveformat->cbSize = 0;

    if(waveOutOpen(&(opaque->hWaveOut), WAVE_MAPPER, waveformat, (DWORD_PTR)waveOutProc, (DWORD_PTR)opaque, CALLBACK_FUNCTION) !=MMSYSERR_NOERROR) {
        LOGE("aout_open_audio: unable to openWAVE_MAPPER device");
        return -1;
    }

    opaque->buffer_size = waveformat->nAvgBytesPerSec;

    opaque->buffer = (uint8_t*)malloc(opaque->buffer_size);
    if (!opaque->buffer) {
        LOGE("aout_open_audio: failed to allocate buffer");
        waveOutClose(opaque->hWaveOut);
        return -1;
    }

    if (obtained) {
        //just copy
        *obtained = *desired;
        obtained->size = opaque->buffer_size;
        LOGW("audio target format fmt:0x%x, channel:0x%x",
                (int) obtained->format, (int) obtained->channels);
    }

    opaque->pause_on = 1;
    opaque->abort_request = 0;
    opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread,
            aout, "ff_aout_android");
    if (!opaque->audio_tid) {
        LOGE("aout_open_audio_n: failed to create audio thread");
        waveOutClose(opaque->hWaveOut);
        return -1;
    }

    return 0;
}

void aout_pause_audio(SDL_Aout *aout, int pause_on) {
    SDL_Aout_Opaque *opaque = aout->opaque;

    SDL_LockMutex(opaque->wakeup_mutex);
    LOGW("aout_pause_audio(%d)", pause_on);
    opaque->pause_on = pause_on;
    if (!pause_on)
        SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

void aout_flush_audio(SDL_Aout *aout) {
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_LockMutex(opaque->wakeup_mutex);
    LOGW("aout_flush_audio()");
    opaque->need_flush = 1;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

void aout_set_volume(SDL_Aout *aout, float left_volume, float right_volume) {
    SDL_Aout_Opaque *opaque = aout->opaque;

    if (left_volume < MIN_VOLUME)
    {
        left_volume = MIN_VOLUME;
    }
    if (left_volume > MAX_VOLUME)
    {
        left_volume = MAX_VOLUME;
    }
    if (right_volume < MIN_VOLUME)
    {
        right_volume = MIN_VOLUME;
    }
    if (right_volume > MAX_VOLUME)
    {
        right_volume = MAX_VOLUME;
    }

    SDL_LockMutex(opaque->wakeup_mutex);
    LOGW("aout_set_volume(%f, %f)", left_volume, right_volume);
    //high-order word contains the right-channel setting
    opaque->dwVolume = ((DWORD)(right_volume * FULL_VOLUME) << VOLUME_BIT) + (DWORD)(left_volume * FULL_VOLUME);
    opaque->need_set_volume = 1;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

void aout_close_audio(SDL_Aout *aout) {
    SDL_Aout_Opaque *opaque = aout->opaque;

    SDL_LockMutex(opaque->wakeup_mutex);
    opaque->abort_request = true;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);

    if (opaque->audio_tid)
        SDL_WaitThread(opaque->audio_tid, NULL);

    opaque->audio_tid = NULL;
}

void aout_free_l(SDL_Aout *aout) {
    if (!aout)
        return;

    aout_close_audio(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;
    if (opaque) {
        free(opaque->buffer);
        opaque->buffer = NULL;
        opaque->buffer_size = 0;

        SDL_DestroyCond(opaque->wakeup_cond);
        SDL_DestroyMutex(opaque->wakeup_mutex);
    }

    SDL_Aout_FreeInternal(aout);
}

SDL_Aout *SDL_AoutCreate() {
    SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;

    SDL_Aout_Opaque *opaque = aout->opaque;
    opaque->wakeup_cond = SDL_CreateCond();
    opaque->wakeup_mutex = SDL_CreateMutex();

    aout->free_l = aout_free_l;
    aout->open_audio = aout_open_audio;
    aout->pause_audio = aout_pause_audio;
    aout->flush_audio = aout_flush_audio;
    aout->set_volume = aout_set_volume;
    aout->close_audio = aout_close_audio;

    return aout;
}
