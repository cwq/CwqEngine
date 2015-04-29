#include "FrameQueue.h"

MTAudioFrame::MTAudioFrame() {
	memset(this, 0, sizeof(MTAudioFrame));
}

MTAudioFrame::~MTAudioFrame() {
	if (avFrame) {
		av_frame_free(&avFrame);
	}
}

void MTAudioFrame::releaseResource() {
	if (avFrame) {
		//释放软解时占用的资源
		av_frame_unref(avFrame);
	}

	data = NULL;
}

void MTAudioFrame::copyFromAvFrame(AVFrame *srcFrame) {
	if (!avFrame) {
		avFrame = av_frame_alloc();
	}
	av_frame_move_ref(avFrame, srcFrame);

	sample_rate = avFrame->sample_rate;
	nb_samples = avFrame->nb_samples;
	channels = av_frame_get_channels(avFrame);
	channel_layout = avFrame->channel_layout;
	format = (AVSampleFormat) avFrame->format;
	data = avFrame->extended_data == NULL ?
			avFrame->data : avFrame->extended_data;

}

void frame_queue_unref_item(Frame *vp) {
	vp->mtAudioFrame.releaseResource();
#ifdef FFP_MERGE
	avsubtitle_free(&vp->sub);
#endif
}

int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size,
		int keep_last) {
	memset(f, 0, sizeof(FrameQueue));
//	if (!(f->mutex = SDL_CreateMutex()))
//		return AVERROR(ENOMEM);
	if (!(f->r_cond = SDL_CreateCond()))
		return AVERROR(ENOMEM);
	f->pktq = pktq;
	f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
	f->keep_last = !!keep_last;
	return 0;
}

void frame_queue_set_locker(FrameQueue *f, SDL_mutex *m, SDL_cond *c) {
	f->mutex = m;
	f->cond = c;
}

void frame_queue_destory(FrameQueue *f) {
	int i;
	for (i = 0; i < f->max_size; i++) {
		Frame *vp = &f->queue[i];
		frame_queue_unref_item(vp);
	}
//	SDL_DestroyMutex(f->mutex);
	SDL_DestroyCond(f->r_cond);
}

void frame_queue_signal(FrameQueue *f) {
	SDL_LockMutex(f->mutex);
	SDL_CondSignal(f->r_cond);
	SDL_UnlockMutex(f->mutex);
}

Frame *frame_queue_peek(FrameQueue *f) {
	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

Frame *frame_queue_peek_next(FrameQueue *f) {
	return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

Frame *frame_queue_peek_last(FrameQueue *f) {
	return &f->queue[f->rindex];
}

Frame *frame_queue_peek_writable(FrameQueue *f) {
	/* wait until we have space to put a new frame */
//	SDL_LockMutex(f->mutex);
//	while (f->size >= f->max_size && !f->pktq->abort_request) {
//		SDL_CondWait(f->cond, f->mutex);
//	}
//	SDL_UnlockMutex(f->mutex);
	if (f->pktq->abort_request)
		return NULL;

	return &f->queue[f->windex];
}

Frame *frame_queue_peek_readable(FrameQueue *f) {
	/* wait until we have a readable a new frame */
	SDL_LockMutex(f->mutex);
	while (f->size - f->rindex_shown <= 0 && !f->pktq->abort_request) {
		SDL_CondWait(f->r_cond, f->mutex);
	}
	SDL_UnlockMutex(f->mutex);

	if (f->pktq->abort_request)
		return NULL;

	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

void frame_queue_push(FrameQueue *f) {
	if (++f->windex == f->max_size)
		f->windex = 0;
	SDL_LockMutex(f->mutex);
	f->size++;
	SDL_CondSignal(f->r_cond);
	SDL_UnlockMutex(f->mutex);
}

void frame_queue_next(FrameQueue *f) {
	if (f->keep_last && !f->rindex_shown) {
		f->rindex_shown = 1;
		return;
	}
	frame_queue_unref_item(&f->queue[f->rindex]);
	if (++f->rindex == f->max_size)
		f->rindex = 0;
	SDL_LockMutex(f->mutex);
	f->size--;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}

/* jump back to the previous frame if available by resetting rindex_shown */
int frame_queue_prev(FrameQueue *f) {
	int ret = f->rindex_shown;
	f->rindex_shown = 0;
	return ret;
}

/* return the number of undisplayed frames in the queue */
int frame_queue_nb_remaining(FrameQueue *f) {
	return f->size - f->rindex_shown;
}

bool frame_queue_is_full(FrameQueue *f) {
	return f->size >= f->max_size;
}

