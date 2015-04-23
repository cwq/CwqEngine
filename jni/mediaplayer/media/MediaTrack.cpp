#include "MediaTrack.h"

MediaTrack::MediaTrack(int startTime) {
	mediaDecoder = NULL;
	this->startTime = startTime;
}

MediaTrack::~MediaTrack() {
	if (mediaDecoder != NULL) {
		delete mediaDecoder;
	}
}

bool MediaTrack::addDecoder(MediaDecoder *mediaDecoder) {
	this->mediaDecoder = mediaDecoder;
	return true;
}

void MediaTrack::packetQueuAbort() {
	mediaDecoder->packetQueuAbort();
}

void MediaTrack::close() {
	mediaDecoder->close();
}

bool MediaTrack::isTimeValid(int wantedTime) {
	return (startTime <= wantedTime)
			&& (wantedTime <= (startTime + mediaDecoder->getTimeLength()));
}

ReadPktStatus MediaTrack::readPkt() {
	return mediaDecoder->readPkt();
}

bool MediaTrack::hasVideo() {
	return mediaDecoder->hasVideo();
}

bool MediaTrack::hasAudio() {
	return mediaDecoder->hasAudio();
}

DecodePktStatus MediaTrack::decodeAudio() {
	return mediaDecoder->decodeAudio();
}

int MediaTrack::getPCM() {
	return 0;
}

DecodePktStatus MediaTrack::decodeVideo() {
	return mediaDecoder->decodeVideo();
}

VideoPicture *MediaTrack::getNextFrame(int *remaingTimes) {
	return mediaDecoder->getNextFrame(remaingTimes);
}

MediaDecoder *MediaTrack::getMediaDecoder() {
    return mediaDecoder;
}

void MediaTrack::setPause() {
	mediaDecoder->setPause();
}

bool MediaTrack::isPaused() {
	return mediaDecoder->isPaused();
}

int MediaTrack::seekTo(int64_t tragetPos, int64_t minPos, int64_t maxPos,
		bool isPlaying) {
	return mediaDecoder->seekTo(tragetPos, minPos, maxPos, isPlaying);
}

int MediaTrack::seekTo(int value, bool seekByTime, bool isPlaying) {
	return mediaDecoder->seekTo(value, seekByTime, isPlaying);
}

void MediaTrack::togglePause() {
	mediaDecoder->togglePause();
}

int MediaTrack::getPlayedTime() {
	return mediaDecoder->getPlayedTime();
}
