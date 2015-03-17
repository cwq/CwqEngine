#ifndef __PACKET_QUEUE_H__
#define __PACKET_QUEUE_H__

#include "audio/AudioDefine.h"
#include "PacketQueue.h"

class MTAudioFrame {
public:
	MTAudioFrame();
	~MTAudioFrame();

	//用完数据后，释放内存资源，比如AVFrame用完后要unref，就在这里做
	void releaseResource();
	//从AVFrame拷贝数据，软件时使用
	void copyFromAvFrame(AVFrame *avFrame);

	//软解需要用avFrame保存音频帧数据，以便此帧数据用完后，使用av_frame_unref()直接释放
	AVFrame *avFrame;

	int channels;
	/**
	 * Channel layout of the audio data.
	 */
	int64_t channel_layout;
	/**
	 * number of audio samples (per channel) described by this frame
	 */
	int nb_samples;
	AVSampleFormat format;
	/**
	 * Sample rate of the audio data.
	 */
	int sample_rate;

	//PCM数据缓冲区
	uint8_t **data;
};

typedef struct Frame {
	MTAudioFrame mtAudioFrame;
	AVSubtitle sub;
	int serial;
	double pts; /* presentation timestamp for the frame */
	double duration; /* estimated duration of the frame */
	int64_t pos; /* byte position of the frame in the input file */
	int allocated;
	int reallocate;
	int width;
	int height;
	AVRational sar;
} Frame;

typedef struct FrameQueue {
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	int keep_last;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	SDL_cond *r_cond;
	PacketQueue *pktq;
} FrameQueue;

void frame_queue_unref_item(Frame *vp);
int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size,
		int keep_last);
void frame_queue_set_locker(FrameQueue *f, SDL_mutex *m, SDL_cond *c);
void frame_queue_destory(FrameQueue *f);
void frame_queue_destory(FrameQueue *f);
void frame_queue_signal(FrameQueue *f);
Frame *frame_queue_peek(FrameQueue *f);
Frame *frame_queue_peek(FrameQueue *f);
Frame *frame_queue_peek_next(FrameQueue *f);
Frame *frame_queue_peek_last(FrameQueue *f);
Frame *frame_queue_peek_writable(FrameQueue *f);
Frame *frame_queue_peek_readable(FrameQueue *f);
void frame_queue_push(FrameQueue *f);
void frame_queue_next(FrameQueue *f);
int frame_queue_prev(FrameQueue *f);
int frame_queue_nb_remaining(FrameQueue *f);
bool frame_queue_is_full(FrameQueue *f);

#endif
