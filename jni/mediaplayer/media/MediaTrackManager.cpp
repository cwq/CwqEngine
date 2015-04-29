#include "MediaTrackManager.h"

bool MediaTrackManager::hasVideo() {
	MediaTrack *track;
	LIST_FOR_EACH_TRACK(track, this)
	{
		if (track->hasVideo()) {
			return true;
		}
	}
	return false;
}

bool MediaTrackManager::hasAudio() {
	MediaTrack *track;
	LIST_FOR_EACH_TRACK(track, this)
	{
		if (track->hasAudio()) {
			return true;
		}
	}
	return false;
}

bool MediaTrackManager::isPaused() {
	MediaTrack *mediaTrack;
	bool isPaused = true;
	LIST_FOR_EACH_TRACK(mediaTrack, this)
	{
		isPaused &= mediaTrack->isPaused();
	}
	return isPaused;
}
