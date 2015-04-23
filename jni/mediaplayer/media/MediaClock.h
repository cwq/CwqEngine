#ifndef __MEDIA_CLOCK_H__
#define __MEDIA_CLOCK_H__

#define AV_NOSYNC_THRESHOLD 10.0

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.01
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.2
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

enum AvSyncType {
	AV_SYNC_AUDIO_MASTER, AV_SYNC_VIDEO_MASTER, AV_SYNC_EXTERNAL_CLOCK
};

class MediaClock {
public:
	MediaClock(int *queue_serial);
	void set_clock(double pts, int serial); //设置新时钟值
	void set_clock(double pts, int serial, double time);
	double get_clock(); //获取当前时钟值
	void sync_clock_to(MediaClock *slave); //同步到slave时钟
	void set_clock_speed(double speed); //设置时钟速率
	void clock_speed_up(); //时钟加速
	void clock_speed_down(); //减速
	void clock_speed_reset(); //重置速度

	void reset(int *queue_serial);

	double pts; /* clock base */
	double pts_drift; /* clock base minus time at which we updated the clock */
	double last_updated;
	double speed;
	int serial; /* clock is based on a packet with this serial */
	bool paused;
	int *queue_serial; /* pointer to the current packet queue serial, used for obsolete clock detection */
};

#endif

