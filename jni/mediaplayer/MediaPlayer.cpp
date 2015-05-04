#include <assert.h>
#include "ijkutil/cross_sleep.h"
#include "MediaPlayer.h"
#include "audio/audio_mixer.h"
#include "base/LogHelper.h"
#include "renderer/Image.h"

static void copyFromAVFrame(u_char *pixels, AVFrame *frame, int width, int height) {
    if (!frame || !pixels) {
        return;
    }

    int y = 0;
    int numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);

    for (y = 0; y < height; y++) {
        memcpy(pixels + (y * width * 3), frame->data[0] + y * frame->linesize[0], width * 3);
    }
}

Callbacker::Callbacker(MediaPlayer *mediaPlayer) {
	assert(mediaPlayer);
	this->mediaPlayer = mediaPlayer;
}

void Callbacker::lock() {
	SDL_LockMutex(mediaPlayer->pictq_mutex);
}

void Callbacker::unlock() {
	SDL_UnlockMutex(mediaPlayer->pictq_mutex);
}

MediaPlayer::MediaPlayer() {
	pictq_mutex = SDL_CreateMutex();
	pictq_cond = SDL_CreateCond();
	continue_read_thread = SDL_CreateCond();

	//audio
	audioPlayer = new AudioPlayer();
	frameQueueMutex = SDL_CreateMutex();
	frameQueueCond = SDL_CreateCond();

	callbacker = new Callbacker(this);
	autoExit = false;
}

MediaPlayer::~MediaPlayer() {
	SDL_DestroyCond(continue_read_thread);
	SDL_DestroyMutex(pictq_mutex);
	SDL_DestroyCond(pictq_cond);

	MediaTrack *mediaTrack;
	LIST_FOR_EACH_TRACK(mediaTrack, &mediaTrackManager)
	{
		delete mediaTrack;
	}

	if (callbacker != NULL) {
		delete callbacker;
	}

	//audio
	if (audioPlayer != NULL) {
		delete audioPlayer;
		audioPlayer = NULL;
	}
	SDL_DestroyMutex(frameQueueMutex);
	SDL_DestroyCond(frameQueueCond);
}

static int lockmgr(void **mtx, enum AVLockOp op) {
	switch (op) {
	case AV_LOCK_CREATE:
		*mtx = SDL_CreateMutex();
		if (!*mtx)
			return 1;
		return 0;
	case AV_LOCK_OBTAIN:
		return !!SDL_LockMutex(*(SDL_mutex**) mtx);
	case AV_LOCK_RELEASE:
		return !!SDL_UnlockMutex(*(SDL_mutex**) mtx);
	case AV_LOCK_DESTROY:
		SDL_DestroyMutex(*(SDL_mutex**) mtx);
		return 0;
	}
	return 1;
}

static void ffp_log_callback_report(void *ptr, int level, const char *fmt,
		va_list vl) {
	va_list vl2;
    char line[1024];
    static int print_prefix = 1;

    vl2 = vl;
    //va_copy(vl2, vl);
    // av_log_default_callback(ptr, level, fmt, vl);
    av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
    //va_end(vl2);

	if (level <= AV_LOG_ERROR)
	    {LOGE("%s", line);}
	else if (level <= AV_LOG_WARNING)
	    {LOGW("%s", line);}
	else if (level <= AV_LOG_INFO)
	    {LOGI("%s", line);}
	else if (level <= AV_LOG_VERBOSE)
	    {LOGV("%s", line);}
	else
	    {LOGD("%s", line);}
}

void MediaPlayer::init() {
	LOGD("MediaPlayer init");

	AvSyncType syncType = AV_SYNC_EXTERNAL_CLOCK;
	avSyncType = syncType;
	wantToExit = false;
	abort_request = false;

	int frameDrop = 0;
	this->frameDrop = frameDrop;

	int seekByBytes = -1;
	this->seekByBytes = seekByBytes;
	seek_req = false;

	/* register all codecs, demux and protocols */
	avcodec_register_all();
	av_register_all();

	av_lockmgr_register(lockmgr);
	av_log_set_callback(ffp_log_callback_report);
}

static int video_thread(void *arg) {
	//LOGD("video_thread pid = %u", gettid());
	MediaPlayer *mediaPlayer = (MediaPlayer *) arg;
	bool anyOneDecode = false, allTrackDecodeEnd = true;

	MediaTrack *mediaTrack = nullptr;
	DecodePktStatus status;
	while (true) {
		while (mediaPlayer->mediaTrackManager.isPaused()
				&& !mediaPlayer->abort_request) {
			//LOGD("video_thread block: paused");
			sleep_ms(10);
		}

		anyOneDecode = false;
		allTrackDecodeEnd = true;

		LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
		{
			if (mediaTrack->hasVideo()) {
				status = mediaTrack->decodeVideo();
				if (status == DECODE_PKT_OK) {
					anyOneDecode = true;
				}
				allTrackDecodeEnd &= (status == DECODE_PKT_END);
			}

		}

		if (allTrackDecodeEnd) {
			break;
		}

		/* if the queue are full, no need to read more */
		if (!anyOneDecode) {
			/* wait 10 ms */
//			LOGE("video_thread block: nobody decode");
			SDL_LockMutex(mediaPlayer->pictq_mutex);
			SDL_CondWaitTimeout(mediaPlayer->pictq_cond,
					mediaPlayer->pictq_mutex, 10);
			SDL_UnlockMutex(mediaPlayer->pictq_mutex);
//			LOGE("video_thread unblock");
			continue;
		}
	}

	LOGD("video_thread exit.............");
	return 0;
}

static int decode_interrupt_cb(void *ctx) {
	MediaPlayer *mediaPlayer = (MediaPlayer *) ctx;
	return mediaPlayer->wantToExit;
}

static int audio_thread(void *arg) {
	//LOGD("pthread audio_thread pid = %u", gettid());
	MediaPlayer *mediaPlayer = (MediaPlayer *) arg;
	bool anyOneDecode = false, allTrackDecodeEnd = true;

	MediaTrack *mediaTrack = nullptr;
	DecodePktStatus status;
	while (true) {
		if (mediaPlayer->isAbortRequest()) {
			break;
		}

		anyOneDecode = false;
		allTrackDecodeEnd = true;

		LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
		{
			if (mediaTrack->hasAudio()) {
				status = mediaTrack->decodeAudio();
				if (status == DECODE_PKT_OK) {
					anyOneDecode = true;
				}
				allTrackDecodeEnd &= (status == DECODE_PKT_END);
			}
		}

		if (allTrackDecodeEnd) {
			break;
		}

		/* if the queue are full, no need to read more */
		if (!anyOneDecode) {
			/* wait 10 ms */
//			LOGE("cwq audio_thread block: nobody decode");
//			sleep_ms(10);
			SDL_LockMutex(mediaPlayer->frameQueueMutex);
			SDL_CondWaitTimeout(mediaPlayer->frameQueueCond, mediaPlayer->frameQueueMutex, 10);
			SDL_UnlockMutex(mediaPlayer->frameQueueMutex);
//			LOGE("cwq audio_thread unblock");
			continue;
		}
	}

	LOGD("aaaaaa audio thread exit.............");
	return 0;
}

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len) {
	MediaPlayer *mediaPlayer = (MediaPlayer *) opaque;
	MediaTrack *mediaTrack = nullptr;
	MediaDecoder *audioDecoder = nullptr;
	Uint8 *tempStream = (Uint8 *) malloc(len);
	int audioNum = 0;

	LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
	{
		if (mediaTrack->hasAudio()) {
			++audioNum;
			audioDecoder = mediaTrack->getMediaDecoder();
			int templen = len;
			int audio_size, len1;

			while (templen > 0) {
				if (audioDecoder->audio_buf_index
						>= audioDecoder->audio_buf_size) {
					audio_size = audioDecoder->audio_decode_frame();
					if (audio_size < 0) {
						/* if error, just output silence */
						audioDecoder->audio_buf = audioDecoder->silence_buf;
						audioDecoder->audio_buf_size =
								sizeof(audioDecoder->silence_buf)
										/ audioDecoder->audio_tgt.frame_size
										* audioDecoder->audio_tgt.frame_size;
					} else {
						audioDecoder->audio_buf_size = audio_size;
					}
					audioDecoder->audio_buf_index = 0;
				}
				if (audioDecoder->auddec.pkt_serial
						!= audioDecoder->audioq.serial) {
					// LOGE("aout_cb: flush\n");
					audioDecoder->audio_buf_index =
							audioDecoder->audio_buf_size;
					memset(tempStream, 0, templen);
					tempStream += templen;
					templen = 0;
					mediaPlayer->audioPlayer->flush();
					break;
				}
				len1 = audioDecoder->audio_buf_size
						- audioDecoder->audio_buf_index;
				if (len1 > templen)
					len1 = templen;
				memcpy(tempStream,
						(uint8_t *) audioDecoder->audio_buf
								+ audioDecoder->audio_buf_index, len1);
				templen -= len1;
				tempStream += len1;
				audioDecoder->audio_buf_index += len1;
			}
			audioDecoder->audio_write_buf_size = audioDecoder->audio_buf_size
					- audioDecoder->audio_buf_index;

			tempStream -= len;
			if (audioNum == 1) {
				memcpy(stream, tempStream, len);
			} else {
				AUDIO_MixAudioFormat(stream, tempStream, len,
						AUDIO_MIX_MAXVOLUME);
			}
		}
	}
	//no audio
	if (audioNum == 0)
		memset(stream, 0, len);

	stream += len;
}

int MediaPlayer::audio_open(int64_t wanted_channel_layout,
		int wanted_nb_channels, int wanted_sample_rate,
		struct AudioParams *audio_hw_params) {
	const char *env;
	SDL_AudioSpec wanted_spec, spec;
	static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
	static const int next_sample_rates[] = { 6000, 11025, 12000, 22050, 24000,
			44100, 48000 };
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

	env = NULL;
	if (env) {
		wanted_nb_channels = atoi(env);
		wanted_channel_layout = av_get_default_channel_layout(
				wanted_nb_channels);
	}
	if (!wanted_channel_layout
			|| wanted_nb_channels
					!= av_get_channel_layout_nb_channels(
							wanted_channel_layout)) {
		wanted_channel_layout = av_get_default_channel_layout(
				wanted_nb_channels);
		wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	}
	wanted_nb_channels = av_get_channel_layout_nb_channels(
			wanted_channel_layout);
	wanted_spec.channels = wanted_nb_channels;
	wanted_spec.freq = wanted_sample_rate;
	if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
		av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
		return -1;
	}
	while (next_sample_rate_idx
			&& next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;
	wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE,
			2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	wanted_spec.callback = sdl_audio_callback;
	wanted_spec.userdata = this;
	while (audioPlayer->open(&wanted_spec, &spec) < 0) {
		av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz)\n",
				wanted_spec.channels, wanted_spec.freq);
		wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
		if (!wanted_spec.channels) {
			wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
			wanted_spec.channels = wanted_nb_channels;
			if (!wanted_spec.freq) {
				av_log(NULL, AV_LOG_ERROR,
						"No more combinations to try, audio open failed\n");
				return -1;
			}
		}
		wanted_channel_layout = av_get_default_channel_layout(
				wanted_spec.channels);
	}
	if (spec.format != AUDIO_S16SYS) {
		av_log(NULL, AV_LOG_ERROR,
				"SDL advised audio format %d is not supported!\n", spec.format);
		return -1;
	}
	if (spec.channels != wanted_spec.channels) {
		wanted_channel_layout = av_get_default_channel_layout(spec.channels);
		if (!wanted_channel_layout) {
			av_log(NULL, AV_LOG_ERROR,
					"SDL advised channel count %d is not supported!\n",
					spec.channels);
			return -1;
		}
	}

	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
	audio_hw_params->freq = spec.freq;
	audio_hw_params->channel_layout = wanted_channel_layout;
	audio_hw_params->channels = spec.channels;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL,
			audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL,
			audio_hw_params->channels, audio_hw_params->freq,
			audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0
			|| audio_hw_params->frame_size <= 0) {
		av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
		return -1;
	}
	return spec.size;
}

void MediaPlayer::addMvTrack(const char *fileName, int startTime, int endTime, int startPlayTime, bool loop) {
	assert(fileName);

    MediaFileInfo* fileInfo = new MediaFileInfo();
    fileInfo->seekByBytes = seekByBytes;
    fileInfo->frameDrop = needToDropFrame();
    fileInfo->avSyncType = getMasterSyncType();

    fileInfo->fileName = fileName;

    if (fileName == this->fileName.c_str()) {
        vector<MediaFileInfo *>::iterator it;
        it = vectorFileInfo.begin();
        vectorFileInfo.insert(it, fileInfo);
    } else {
        vectorFileInfo.push_back(fileInfo);
    }
}

void MediaPlayer::addAllTracks() {
    for(int i = 0; i < vectorFileInfo.size(); ++i) {
        MediaDecoder *mediaDecoder = new MediaDecoder();
        mediaDecoder->setFrameQueueLocker(frameQueueMutex, frameQueueCond);
        mediaDecoder->continue_read_thread = continue_read_thread;
        mediaDecoder->setDecodeInterruptCb(decode_interrupt_cb, this);
        mediaDecoder->setMediaDecodeCallbacker(callbacker);

        if (mediaDecoder->open(*vectorFileInfo[i]) < 0) {
            LOGE("read_thread mediaDecoder failed");
            return;
        }

        MediaTrack *mediaTrack = new MediaTrack(0);
        mediaTrack->addDecoder(mediaDecoder);
        mediaDecoder->audio_tgt = *audioPlayer->getAudioParams();

        mediaTrackManager.addTrack(mediaTrack);
    }
}

static int read_thread(void *arg) {
	//LOGD("read_thread pid = %u", gettid());
	MediaPlayer *mediaPlayer = (MediaPlayer *) arg;
	MediaTrack *mediaTrack;

    mediaPlayer->addAllTracks();

	if (mediaPlayer->mediaTrackManager.hasVideo()) {
		mediaPlayer->video_tid = SDL_CreateThreadEx(&mediaPlayer->_video_tid,
				video_thread, mediaPlayer, "video_decode");
	}

	if (mediaPlayer->mediaTrackManager.hasAudio()) {
		mediaPlayer->audio_tid = SDL_CreateThreadEx(&mediaPlayer->_audio_tid,
				audio_thread, mediaPlayer, "audio_decode");
	}

	SDL_mutex *wait_mutex = SDL_CreateMutex();

	while (true) {
		if (mediaPlayer->wantToExit) {
			break;
		}

		LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
		{
			mediaTrack->setPause();
		}

		if (mediaPlayer->seek_req) {
			LOGD("ttt process msg seek");
			LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
			{
				mediaTrack->seekTo(mediaPlayer->seek_value,
						mediaPlayer->seek_by_time, true);
			}
			mediaPlayer->seek_req = false;
		}

		bool anyOneRead = false, allTrackDecodeEnd = true;
		LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
		{
			ReadPktStatus status = mediaTrack->readPkt();
			anyOneRead |= (status == READ_PKT_OK);
			allTrackDecodeEnd &= (status == READ_PKT_END);
		}

		if (allTrackDecodeEnd && mediaPlayer->autoExit) {
			break;
		}

		/* if the queue are full, no need to read more */
		if (!anyOneRead) {
//			LOGE("read_thread block: nobody read");
			/* wait 10 ms */
			SDL_LockMutex(wait_mutex);
			SDL_CondWaitTimeout(mediaPlayer->continue_read_thread, wait_mutex,
					10);
			SDL_UnlockMutex(wait_mutex);
//			LOGE("read_thread unblock");
			continue;
		}
	}
	LOGD("read_thread block: wait for abort");
	while (!mediaPlayer->wantToExit) {
	    sleep_ms(1);
	}
	LOGD("read_thread unblock");

	LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
	{
		//唤醒因无包可读而处于沉睡中的viedeo_thread，abort_request置true防止video_thread因paused进入死循环
		mediaPlayer->abort_request = true;
		mediaTrack->packetQueuAbort();
	}

	/* note: we also signal this mutex to make sure we deblock the
	 video thread in all cases */
	SDL_LockMutex(mediaPlayer->pictq_mutex);
	SDL_CondSignal(mediaPlayer->pictq_cond);
	SDL_UnlockMutex(mediaPlayer->pictq_mutex);

	LOGD("read_thread block: wait for video_thread exit");
	if(mediaPlayer->video_tid)
	    SDL_WaitThread(mediaPlayer->video_tid, NULL);
	LOGD("read_thread unblock");

    //deblock the audio thread
    SDL_LockMutex(mediaPlayer->frameQueueMutex);
    SDL_CondSignal(mediaPlayer->frameQueueCond);
    SDL_UnlockMutex(mediaPlayer->frameQueueMutex);

	LOGD("read_thread block: wait for audio_thread exit");
	if(mediaPlayer->audio_tid)
	    SDL_WaitThread(mediaPlayer->audio_tid, NULL);
	LOGD("read_thread unblock");

    LIST_FOR_EACH_TRACK(mediaTrack, &mediaPlayer->mediaTrackManager)
    {
        mediaTrack->close();
    }

	SDL_DestroyMutex(wait_mutex);

	LOGD("read_thread exit");

	return 0;
}

bool MediaPlayer::needToDropFrame() {
	return (frameDrop > 0
			|| (frameDrop && getMasterSyncType() != AV_SYNC_VIDEO_MASTER));
}

void MediaPlayer::toggle_pause() {
	MediaTrack *mediaTrack;
	LIST_FOR_EACH_TRACK(mediaTrack, &mediaTrackManager)
	{
		mediaTrack->togglePause();
	}
	audioPlayer->pause(mediaTrackManager.isPaused());
}

/* seek in the strem */
void MediaPlayer::stream_seek(int value, bool by_time) {
	if (!seek_req) {
		seek_value = value;
		seek_by_time = by_time;
		seek_req = 1;
		SDL_CondSignal(continue_read_thread);
	}
}

bool MediaPlayer::processMessage() {
	AVMessage msg;
	int ret = msgQueue.get(&msg, 0);
	if (ret <= 0) {
		//没有消息
		return false;
	}

	switch (msg.what) {
	case FFP_REQ_PAUSE:
		toggle_pause();
		break;

	case FFP_REQ_SEEK:
		stream_seek(msg.arg1, msg.arg2);
		break;
	}

	return true;
}

void MediaPlayer::getNextFrame(int *remaingTimes, const vector<Image *> &images) {
	processMessage();

//	LOGE("draw_thread pid = %u", gettid());
	MediaTrack *mediaTrack;
	int index = 0;
	VideoPicture* vp = NULL;
	LIST_FOR_EACH_TRACK(mediaTrack, &mediaTrackManager)
	{
		if (mediaTrack->hasVideo()) {
			int trackRemaingTimes = *remaingTimes;
			vp = mediaTrack->getNextFrame(&trackRemaingTimes);
			if(vp != NULL) {
			    images[index]->initWithImageInfo(vp->width, vp->height, GL_RGB);
			    copyFromAVFrame((u_char*)images[index]->getPixels(), vp->decodedFrame, vp->width, vp->height);
			}
			++index;
			*remaingTimes = FFMIN(*remaingTimes, trackRemaingTimes);
			SDL_CondSignal(pictq_cond);
		}
	}

	processMessage();
}

void MediaPlayer::start() {
	init();

	msgQueue.start();

	//添加track
	if(!fileName.empty())
        addMvTrack(fileName.c_str(), 0, 0, 0, false);
    //第二层track
//    addMvTrack("/sdcard/ninja.mp4", 0, 0, 0, false);
//    addMvTrack("/sdcard/test.mp3", 0, 0, 0, false);

    //open audio
    int ret;
    if ((ret = audio_open(audioPlayer->getChannelLayout(), audioPlayer->getChannels(), audioPlayer->getSampleRate(), audioPlayer->getAudioParams())) < 0) {
        LOGE("xxx audio_open error");
    }
    //start audio
    audioPlayer->pause(0);

	read_tid = SDL_CreateThreadEx(&_read_tid, read_thread, this, "video_read");
	assert(read_tid);
}

void MediaPlayer::end() {
	LOGD("mediaplayer end");
	wantToExit = true;
	SDL_WaitThread(read_tid, NULL);

    //clear vectorFileInfo
    vectorFileInfo.clear();

	//clear tracks
	MediaTrack *mediaTrack;
    LIST_FOR_EACH_TRACK(mediaTrack, &mediaTrackManager)
    {
        delete mediaTrack;
    }
    mediaTrackManager.clearTracks();

	//stop audio
	audioPlayer->close();

	av_lockmgr_register(NULL);
	avformat_network_deinit();
	av_log(NULL, AV_LOG_QUIET, "%s", "");

	msgQueue.abort();
}

AvSyncType MediaPlayer::getMasterSyncType() {
	if (avSyncType == AV_SYNC_VIDEO_MASTER) {
		if (mediaTrackManager.hasVideo())
			return AV_SYNC_VIDEO_MASTER;
	} else if (avSyncType == AV_SYNC_AUDIO_MASTER) {
		if (mediaTrackManager.hasAudio())
			return AV_SYNC_AUDIO_MASTER;
	}
	return AV_SYNC_EXTERNAL_CLOCK;
}

void MediaPlayer::setBaselineVideo(const char *fileName) {
	this->fileName = fileName;
}

void MediaPlayer::reset() {
	// 停止播放
	this->end();
}

void MediaPlayer::togglePause() {
	msgQueue.remove(FFP_REQ_PAUSE);
	msgQueue.put_simple1(FFP_REQ_PAUSE);
}

void MediaPlayer::pause() {
    if(!mediaTrackManager.isPaused()) {
        toggle_pause();
    }
}

void MediaPlayer::resume() {
    if(mediaTrackManager.isPaused()) {
        toggle_pause();
    }
}

void MediaPlayer::seekByPercent(int percent) {
	percent = FFMAX(0, FFMIN(percent, 100));
	msgQueue.remove(FFP_REQ_SEEK);
	msgQueue.put_simple3(FFP_REQ_SEEK, percent, false);
}

void MediaPlayer::seekByTime(int time) {
	time = FFMAX(0, time);
	msgQueue.remove(FFP_REQ_SEEK);
	msgQueue.put_simple3(FFP_REQ_SEEK, time, true);
}

int MediaPlayer::getPlayedTime() {
	MediaTrack *mediaTrack;
	LIST_FOR_EACH_TRACK(mediaTrack, &mediaTrackManager)
	{
		return mediaTrack->getPlayedTime();
	}
	return -1;
}

