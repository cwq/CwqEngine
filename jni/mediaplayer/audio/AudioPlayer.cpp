#include "AudioPlayer.h"
#include "base/LogHelper.h"

AudioPlayer::AudioPlayer() {
	aout = SDL_AoutCreate();
	if (aout == NULL) {
		LOGE("Could not initialize AudioPlayer\n");
	}

    channel_layout = 1;
    channels = 1;
    sample_rate = 44100;
}

AudioPlayer::~AudioPlayer() {
	if (aout != NULL) {
		SDL_AoutFree(aout);
		aout = NULL;
	}
}

int AudioPlayer::open(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
	return SDL_AoutOpenAudio(aout, desired, obtained);
}

void AudioPlayer::pause(int pause_on) {
	SDL_AoutPauseAudio(aout, pause_on);
}

void AudioPlayer::flush() {
	SDL_AoutFlushAudio(aout);
}

void AudioPlayer::setStereoVolume(float left_volume, float right_volume) {
	SDL_AoutSetStereoVolume(aout, left_volume, right_volume);
}

void AudioPlayer::close() {
	SDL_AoutCloseAudio(aout);
}
