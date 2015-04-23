#ifndef __FRAME_QUEUE_H__
#define __FRAME_QUEUE_H__

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

#include "ijksdl_mutex.h"

// 定义最大队列大小
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5

typedef struct MyAVPacketList {
	AVPacket pkt;
	struct MyAVPacketList *next;
	int serial;
} MyAVPacketList;

typedef struct PacketQueue {
	MyAVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int abort_request;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

bool is_flush_pkt(AVPacket *pkt);
void packet_queue_start(PacketQueue *q);
void packet_queue_abort(PacketQueue *q);
void packet_queue_destroy(PacketQueue *q);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);
void packet_queue_flush(PacketQueue *q);
int packet_queue_put_nullpacket(PacketQueue *q, int stream_index);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);
void packet_queue_init(PacketQueue *q);
int packet_queue_put_flush_pkt(PacketQueue *q);

#endif
