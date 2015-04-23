#ifndef __MEDIA_TRACKE_MANAGER_H__
#define __MEDIA_TRACKE_MANAGER_H__

#include <vector>
using std::vector;

#include "../ijkutil/TrackManager.h"

#include "MediaTrack.h"
class MediaTrackManager: public TrackManager<MediaTrack> {
public:
	bool hasVideo();

	bool hasAudio();

	bool isPaused();
};

#endif
