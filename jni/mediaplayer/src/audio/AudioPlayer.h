#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include "AudioDefine.h"
//extern "C" {
#include "ijksdl_aout.h"
//}

class AudioPlayer {
public:
	AudioPlayer();
	~AudioPlayer();

	//打开音频硬件
	int open(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);

	//暂停
	void pause(int pause_on);

	//清掉已缓存但未播放的音频数据
	void flush();

	//设置声道音量
	void setStereoVolume(float left_volume, float right_volume);

	//关闭硬件
	void close();

	struct AudioParams* getAudioParams() { return &audio_tgt; }
	void setAudioParams(struct AudioParams& tgt) { audio_tgt = tgt; }

	int64_t getChannelLayout() const { return channel_layout; }
	void setChannelLayout(int64_t channel) { channel_layout = channel; }

	int getChannels() const { return channels; }
	void setChannels(int channel) { channels = channel; }

	int getSampleRate() const { return sample_rate; }
	void setSampleRate(int rate) { sample_rate = rate; }

private:
	SDL_Aout *aout;
	struct AudioParams audio_tgt;
	int64_t channel_layout;
	int channels;
	int sample_rate;
};

#endif