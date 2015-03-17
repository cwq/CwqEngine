#ifndef __MEDIA_PLAYER_H__
#define __MEDIA_PLAYER_H__

#include <string>
#include "media/MediaClock.h"
#include "media/MediaTrackManager.h"
#include "media/MessageQueue.h"
#include "ijkutil/ijksdl_mutex.h"
#include "ijkutil/ijksdl_thread.h"
#include "audio/AudioPlayer.h"

using std::string;

class MediaPlayer;

class Callbacker: public MediaDecodeCallbacker {

public:
	Callbacker(MediaPlayer *mediaPlayer);

	~Callbacker() {
	}

	void lock();
	void unlock();

protected:
	MediaPlayer *mediaPlayer;
};

class MediaPlayer {
public:
	MediaPlayer();
	~MediaPlayer();

	//创建线程，准备播放
	void start();
	//等待播放完成
	void end();

	const vector<VideoPicture *> &getNextFrame(int *remaingTimes);

	bool needToDropFrame();

	AvSyncType getMasterSyncType();

	void setBaselineVideo(const char *fileName);

	/**
	 *  reset MediaPlayer to IDEL状态
	 */
	void reset();

	inline bool isAbortRequest() {
		return wantToExit;
	}

	void togglePause();

	//根据进度值seek，比如seek到73%，percent值为73
	void seekByPercent(int percent);

	//根据时间seek，单位是毫秒
	void seekByTime(int time);

	bool processMessage();

	void stream_seek(int value, bool by_time);

	//获取当前位置距离视频开始位置的时间长度 毫秒值
	int getPlayedTime();

    /**
     *  添加音/视频track
     *
     *  @param filename 路径
     *  @param startTime 截取的开始时间 毫秒值
     *  @param endTime 截取的结束时间 毫秒值
     *  @param startPlayTime 开始播放track的原视频时间 毫秒值
     *  @param loop 是否循环播放
     */
	void addMvTrack(const char *fileName, int startTime, int endTime, int startPlayTime, bool loop);

    int audio_open(int64_t wanted_channel_layout,
            int wanted_nb_channels, int wanted_sample_rate,
            struct AudioParams *audio_hw_params);

    //添加所有vectorFileInfo中的track
    void addAllTracks();

protected:

    vector<MediaFileInfo *> vectorFileInfo;

	void init();
	void toggle_pause();

public:
	MediaTrackManager mediaTrackManager;

	SDL_mutex *pictq_mutex;
	SDL_cond *pictq_cond;
	SDL_cond *continue_read_thread;
	SDL_Thread _read_tid;
	SDL_Thread *read_tid;
	SDL_Thread _video_tid;
	SDL_Thread *video_tid;
	SDL_Thread _audio_tid;
    SDL_Thread *audio_tid;

	bool wantToExit; //原来ffplay.c里videoState的abort_request
	bool abort_request;

	Callbacker *callbacker;

	AvSyncType avSyncType;
	/*frameDrop取值：
	 * > 0：可以扔帧
	 * < 0: 以画面为同步时钟时，不可以扔帧
	 * = 0: 不可以扔帧
	 */
	int frameDrop;
	string fileName;

	//audio
	AudioPlayer *audioPlayer;
    SDL_mutex *frameQueueMutex;
    SDL_cond *frameQueueCond;

//	MessageQueue<MediaMessage> msgQueue;
	/*seekByBytes取值：
	 * < 0： 取决于视频格式
	 * < 0: 不按字节seek
	 * > 0: 按字节seek
	 */
	int seekByBytes;
	bool seek_req;
	int seek_value;
	bool seek_by_time;

	MessageQueue msgQueue;

	bool autoExit; //播放完后自动退出，如果为true，播放完后再seek就不起作用

	vector<VideoPicture *> vectorFrame;

};

#endif
