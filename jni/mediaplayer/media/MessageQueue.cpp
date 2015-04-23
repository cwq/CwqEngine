extern "C" {
#include "libavutil/mem.h"
}

#include "MessageQueue.h"
#include "base/LogHelper.h"

MessageQueue::MessageQueue() {
	init();
}

MessageQueue::~MessageQueue() {
	destroy();
}

int MessageQueue::put_private(AVMessage *msg) {
	AVMessage *msg1;

	if (abort_request)
		return -1;

#ifdef FFP_MERGE
	msg1 = av_malloc(sizeof(AVMessage));
#else
	msg1 = recycle_msg;
	if (msg1) {
		recycle_msg = msg1->next;
		recycle_count++;
	} else {
		alloc_count++;
		msg1 = (AVMessage *) av_malloc(sizeof(AVMessage));
	}
#ifdef FFP_SHOW_MSG_RECYCLE
	int total_count = recycle_count + alloc_count;
	if (!(total_count % 10)) {
		LOGE("msg-recycle \t%d + \t%d = \t%d\n", recycle_count, alloc_count, total_count);
	}
#endif
#endif
	if (!msg1)
		return -1;
	// LOGE("msg-recycle %d, %d, %d\n", msg->what, msg->arg1, msg->arg2);

	*msg1 = *msg;
	msg1->next = NULL;

	if (!last_msg)
		first_msg = msg1;
	else
		last_msg->next = msg1;
	last_msg = msg1;
	nb_messages++;
	SDL_CondSignal(cond);
	return 0;
}

int MessageQueue::put(AVMessage *msg) {
	int ret;

	SDL_LockMutex(mutex);
	ret = put_private(msg);
	SDL_UnlockMutex(mutex);

	return ret;
}

void MessageQueue::put_simple1(int what) {
	AVMessage msg;
	msg.what = what;
	put(&msg);
}

void MessageQueue::put_simple2(int what, int arg1) {
	AVMessage msg;
	msg.what = what;
	msg.arg1 = arg1;
	put(&msg);
}

void MessageQueue::put_simple3(int what, int arg1, int arg2) {
	AVMessage msg;
	msg.what = what;
	msg.arg1 = arg1;
	msg.arg2 = arg2;
	put(&msg);
}

void MessageQueue::init() {
	memset(this, 0, sizeof(MessageQueue));
	mutex = SDL_CreateMutex();
	cond = SDL_CreateCond();
	abort_request = 1;
}

void MessageQueue::flush() {
	AVMessage *msg, *msg1;

	SDL_LockMutex(mutex);
	for (msg = first_msg; msg != NULL; msg = msg1) {
		msg1 = msg->next;
#ifdef FFP_MERGE
		av_freep(&msg);
#else
		msg->next = recycle_msg;
		recycle_msg = msg;
#endif
	}
	last_msg = NULL;
	first_msg = NULL;
	nb_messages = 0;
	SDL_UnlockMutex(mutex);
}

void MessageQueue::destroy() {
	flush();

	SDL_LockMutex(mutex);
	while (recycle_msg) {
		AVMessage *msg = recycle_msg;
		if (msg)
			recycle_msg = msg->next;
		av_freep(&msg);
	}
	SDL_UnlockMutex(mutex);

	SDL_DestroyMutex(mutex);
	SDL_DestroyCond(cond);
}

void MessageQueue::abort() {
	SDL_LockMutex(mutex);

	abort_request = 1;

	SDL_CondSignal(cond);

	SDL_UnlockMutex(mutex);
}

void MessageQueue::start() {
	SDL_LockMutex(mutex);
	abort_request = 0;

	AVMessage msg;
	msg.what = FFP_MSG_FLUSH;
	put_private(&msg);
	SDL_UnlockMutex(mutex);
}

/* return < 0 if aborted, 0 if no msg and > 0 if msg.  */
int MessageQueue::get(AVMessage *msg, bool block) {
	AVMessage *msg1;
	int ret;

	SDL_LockMutex(mutex);

	for (;;) {
		if (abort_request) {
			LOGE("msg queue is abort");
			ret = -1;
			break;
		}

		msg1 = first_msg;
		if (msg1) {
			first_msg = msg1->next;
			if (!first_msg)
				last_msg = NULL;
			nb_messages--;
			*msg = *msg1;
#ifdef FFP_MERGE
			av_free(msg1);
#else
			msg1->next = recycle_msg;
			recycle_msg = msg1;
#endif
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(cond, mutex);
		}
	}
	SDL_UnlockMutex(mutex);
	return ret;
}

void MessageQueue::remove(int what) {
	AVMessage **p_msg, *msg, *last_msg = first_msg;
	SDL_LockMutex(mutex);

	if (!abort_request && first_msg) {
		p_msg = &first_msg;
		while (*p_msg) {
			msg = *p_msg;

			if (msg->what == what) {
				// LOGE("remove msg %d", msg->what);
				*p_msg = msg->next;
				p_msg = &msg->next;
#ifdef FFP_MERGE
				av_free(msg);
#else
				msg->next = recycle_msg;
				recycle_msg = msg;
#endif
			} else {
				// LOGE("retain msg %d", msg->what);
				last_msg = msg;
				p_msg = &msg->next;
			}
		}

		if (first_msg) {
			this->last_msg = last_msg;
		} else {
			this->last_msg = NULL;
		}
	}

	SDL_UnlockMutex(mutex);
}
