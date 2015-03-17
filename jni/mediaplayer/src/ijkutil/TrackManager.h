#ifndef __TRACKE_MANAGER_H__
#define __TRACKE_MANAGER_H__

#include <vector>
#include <stdio.h>
using std::vector;

template<class T>
class TrackManager {
public:
	TrackManager() {
		vectorTrack.clear();
	}

	void clearTracks() {
	    vectorTrack.clear();
	}

	void addTrack(T *track) {
		vectorTrack.push_back(track);
	}

	//获取track总数
	int getTrackNum() {
		return vectorTrack.size();
	}

	//获取第index个track
	virtual T *getTrack(int index) {
		if (index < 0 || index >= vectorTrack.size()) {
			return NULL;
		}

		return vectorTrack[index];
	}

protected:
	vector<T *> vectorTrack;
};

//遍历每个track
#define LIST_FOR_EACH_TRACK(a, b) \
	for (int i = 0; (a = (b)->getTrack(i)) != NULL; i++)

#endif
