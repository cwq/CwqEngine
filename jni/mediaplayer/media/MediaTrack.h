#ifndef __MEDIA_TRACK_H__
#define __MEDIA_TRACK_H__

#include "MediaDecoder.h"

class MediaTrack {
public:
	MediaTrack(int startTime);
	~MediaTrack();

	bool addDecoder(MediaDecoder *mediaDecoder);

	//读取音频和视频包
	ReadPktStatus readPkt();

	bool hasVideo();

	bool hasAudio();

	//音频解码
	DecodePktStatus decodeAudio();

	//读取音频解码数据
	int getPCM();

	//视频解码
	DecodePktStatus decodeVideo();

	//读取视频解码数据
	VideoPicture *getNextFrame(int *remaingTimes);

	void packetQueuAbort();

	void close();

	MediaDecoder *getMediaDecoder();

	void setPause();

	bool isPaused();

	int seekTo(int64_t tragetPos, int64_t minPos, int64_t maxPos,
			bool isPlaying);

	int seekTo(int value, bool seekByTime, bool isPlaying);

	void togglePause();

	int getPlayedTime();

protected:
	//判断指定时间是否落在track里
	bool isTimeValid(int wantedTime);

protected:
	MediaDecoder *mediaDecoder;
	int startTime, endTime;
};

#endif
