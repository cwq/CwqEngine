#include <math.h>
#include "MediaClock.h"
extern "C" {
#include "libavutil/time.h"
#include "libavutil/common.h"
}

#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

MediaClock::MediaClock(int *queue_serial) {
	reset(queue_serial);
}

void MediaClock::reset(int *queue_serial)
{
    speed = 1.0;
    paused = 0;
    set_clock(NAN, -1);
	this->queue_serial = (queue_serial != NULL) ? queue_serial : &serial;
}

void MediaClock::set_clock(double pts, int serial) {
	double time = av_gettime() / 1000000.0;
	set_clock(pts, serial, time);
}

void MediaClock::set_clock(double pts, int serial, double time) {
	this->pts = pts;
	last_updated = time;
	pts_drift = pts - time;
	this->serial = serial;
}

double MediaClock::get_clock() {
	if (*queue_serial != serial) {
		return NAN;
	}

	if (paused) {
		return pts;
	} else {
		double time = av_gettime() / 1000000.0;
		return pts_drift + time - (time - last_updated) * (1.0 - speed);
	}
}

void MediaClock::set_clock_speed(double speed) {
	set_clock(get_clock(), serial);
	this->speed = speed;
}

void MediaClock::sync_clock_to(MediaClock *slave) {
	double clock = get_clock();
	double slave_clock = slave->get_clock();
	if (!isnan(slave_clock)
			&& (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set_clock(slave_clock, slave->serial);
}

void MediaClock::clock_speed_down() {
	set_clock_speed(
			FFMAX(EXTERNAL_CLOCK_SPEED_MIN, speed - EXTERNAL_CLOCK_SPEED_STEP));
}

void MediaClock::clock_speed_up() {
	set_clock_speed(
			FFMIN(EXTERNAL_CLOCK_SPEED_MAX, speed + EXTERNAL_CLOCK_SPEED_STEP));
}

void MediaClock::clock_speed_reset() {
	if (speed != 1.0)
		set_clock_speed(
				speed
						+ EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed)
								/ fabs(1.0 - speed));
}
