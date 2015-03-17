#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "ijkutil/ijksdl_mutex.h"

#define FFP_MSG_FLUSH                       0
#define FFP_REQ_PAUSE                       20002
#define FFP_REQ_SEEK                        20003

class AVMessage {
public:
	AVMessage() {
		memset(this, 0, sizeof(AVMessage));
	}

	int what;
	int arg1;
	int arg2;
	struct AVMessage *next;
};

class MessageQueue {
public:
	MessageQueue();
	~MessageQueue();

	int put(AVMessage *msg);
	void put_simple1(int what);
	void put_simple2(int what, int arg1);
	void put_simple3(int what, int arg1, int arg2);
	void flush();
	void abort();
	void start();
	int get(AVMessage *msg, bool block);
	void remove(int what);

protected:
	int put_private(AVMessage *msg);
	void init();
	void destroy();

public:
	AVMessage *first_msg, *last_msg;
	int nb_messages;
	int abort_request;
	SDL_mutex *mutex;
	SDL_cond *cond;

	AVMessage *recycle_msg;
	int recycle_count;
	int alloc_count;
};

#endif
