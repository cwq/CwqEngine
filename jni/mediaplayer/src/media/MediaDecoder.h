#ifndef __MEDIA_DECODER_H__
#define __MEDIA_DECODER_H__

#include <string>
#include "ijkutil/PacketQueue.h"
#include "MediaClock.h"
#include "audio/AudioDefine.h"
#include "ijkutil/FrameQueue.h"

using std::string;

#define VIDEO_PICTURE_QUEUE_SIZE 3

typedef struct VideoPicture {
	double pts; // presentation timestamp for this picture
	int64_t pos; // byte position in file
	AVFrame *decodedFrame; //存放解码后的数据
	uint8_t *frameBuffer; //与decodedFrame绑定
	int width, height; /* source height & width */
	int index; //第几帧
	int allocated; //VideoPicture是重复使用的，此值为0表示对象是空闲的
	AVRational sar;
	int serial;
} VideoPicture;

enum ReadPktStatus {
	READ_PKT_END, READ_PKT_OK, READ_PKT_QUEUE_FULL, READ_PKT_EOF, REAK_PKT_ERROR
};

enum DecodePktStatus {
	DECODE_PKT_OK, DECODE_PKT_QUEUE_FULL, DECODE_PKT_END, DECODE_PKT_ERROR, DECODE_PKT_PKTQ_EMPTY
};

class MediaDecodeCallbacker {
public:
	MediaDecodeCallbacker() {
	}
	virtual ~MediaDecodeCallbacker() {
	}

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class MediaFileInfo {
public:
	string fileName;
	bool frameDrop;
	MediaClock *vidclk, *extclk, *audclk;
	AvSyncType avSyncType;
	int seekByBytes;
	int startTime, endTime; //视频开始播放的时间与结束时间

	MediaFileInfo() {
		frameDrop = 1;
		vidclk = extclk = audclk = NULL;
		avSyncType = AV_SYNC_EXTERNAL_CLOCK;
		seekByBytes = false;
	}

	~MediaFileInfo() {
		if (vidclk != NULL) {
			delete vidclk;
		}

		if (extclk != NULL) {
			delete extclk;
		}

		if (audclk != NULL) {
			delete audclk;
		}
	}

	MediaFileInfo& operator =(const MediaFileInfo &fileInfo) {
		memcpy(this, &fileInfo, sizeof(MediaFileInfo));
		return *this;
	}

	double getMasterClock();
};


typedef struct Decoder {
	AVPacket pkt;
	AVPacket pkt_temp;
	PacketQueue *queue;
	AVCodecContext *avctx;
	int pkt_serial;
	int finished;
	int flushed;
	int packet_pending;
	SDL_cond *empty_queue_cond;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
} Decoder;

class MediaDecoder {
public:
	MediaDecoder();
	~MediaDecoder();
	int open(const MediaFileInfo &fielInfo);
	int close();

	//读取音频和视频的包数据
	ReadPktStatus readPkt();

	//解码音频包
	DecodePktStatus decodeAudio();
	//解码视频包
	DecodePktStatus decodeVideo();

	//读取解码后的音频数据
	int getAudioDecodedData();
	//读取解码后的视频数据
	VideoPicture *getNextFrame(int *remaingTimes);

	//获取视频总长度
	int getTimeLength();

	//文件有视频
	bool hasVideo();

	//文件有音频
	bool hasAudio();

	//中断文件读取数据包
	int packetQueuAbort();

	//将rgb存成ppm，仅测试用
	static void saveFrame(AVFrame *pFrame, int width, int height, int iFrame);

	//设置视频解码的中断函数
	void setDecodeInterruptCb(int (*cb)(void *), void *cbParam);

	void setMediaDecodeCallbacker(MediaDecodeCallbacker *callbacker) {
		this->callbacker = callbacker;
	}

	int audio_decode_frame();

	void setFrameQueueLocker(SDL_mutex *frameQueueMutex, SDL_cond *frameQueueCond);

	void setPause();

	bool isPaused();

	//seekWhenPlay为true表示在播放时进行seek，这时要处理时钟等相关状态
	int seekTo(int64_t tragetPos, int64_t minPos, int64_t maxPos,
			bool seekWhenPlay);

	int seekTo(int value, bool seekByTime, bool seekWhenPlay);

	//暂停/恢复播放
	void togglePause();

    //毫秒值
	int getPlayedTime();

protected:
	int stream_component_open(int streamIndex);
	void stream_component_close(int streamIndex);
	bool get_video_frame(AVFrame *frame, AVPacket *pkt, int serial);
	int queue_picture(AVFrame *src_frame, double pts, int64_t pos, int serial);
	void free_picture(VideoPicture *vp);
	void alloc_picture(VideoPicture *vp);
	void pictq_next_picture();
	bool isPictqFull();
	bool isPictqEmpty();
	VideoPicture *choiceNextVideoPicture(int *remaingTimes);
	void update_video_pts(double pts, int64_t pos, int serial);
	double compute_target_delay(double delay);
	void lock() {
		if (callbacker != NULL) {
			callbacker->lock();
		}
	}
	void unlock() {
		if (callbacker != NULL) {
			callbacker->unlock();
		}
	}

	int synchronize_audio(int nb_samples);

	void stream_toggle_pause();

protected:
	int timeLength; //视频长度
	AVFormatContext *pFormatCtx;
	PacketQueue videoq; //视频包队列
	AVStream *videoStream; //视频流
	int st_index[AVMEDIA_TYPE_NB];
	bool fileEof; //文件已读完时为true
	AVFrame *videoSrcFrame; //视频解码时，源数据的frame
	bool videoDecodeEnd; //视频解码结束
	int pictq_size, pictq_rindex, pictq_windex;
	VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE]; //视频帧队列
	struct SwsContext *img_convert_ctx;
	int decoedPictureNum; //已解帧的总数
	MediaDecodeCallbacker *callbacker;
	int pktReadEnd; //包读取结束
	AVPixelFormat decodedFormat;
	MediaFileInfo fileInfo;
	double frame_last_pts; //上一帧的pts
	int64_t frame_last_dropped_pos;
	double frame_last_dropped_pts;
	int frame_drops_early;
	int frame_drops_late;
	int64_t video_current_pos; // current displayed file pos
	double frame_last_duration; //当前帧与上一帧间的pts差值
	int frame_last_dropped_serial;
	double frame_timer;

public:
    // audio
    SDL_cond *continue_read_thread;
    PacketQueue audioq;
    int audio_clock_serial;
    //SDL_AudioSpec wantedSpec;
    unsigned int audio_buf_size; /* in bytes */
    int audio_buf_index; /* in bytes */
    uint8_t *audio_buf;
    unsigned int audio_buf1_size;
    int audio_write_buf_size;
    uint8_t *audio_buf1;
    int audio_hw_buf_size;
    uint8_t silence_buf[SDL_AUDIO_MIN_BUFFER_SIZE];

    struct AudioParams audio_src;
    struct AudioParams audio_tgt;
    int audio_stream;
    AVStream *audioStream;
    Decoder auddec;
    FrameQueue sampq;
    struct SwrContext *swr_ctx;
    //bool paused;

	// maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	double max_frame_duration;
	int read_pause_return;

	bool paused, lastPaused;
	bool step;
};

#endif
