#include "MediaDecoder.h"
#include <assert.h>
#include "base/LogHelper.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/time.h"
}

#define INT64_MAX 9223372036854775807LL
#define INT64_MIN  (-9223372036854775807LL - 1)

double MediaFileInfo::getMasterClock() {
	double val;

	switch (avSyncType) {
	case AV_SYNC_VIDEO_MASTER:
		val = vidclk->get_clock();
		break;
	case AV_SYNC_AUDIO_MASTER:
		val = audclk->get_clock();
		break;
	default:
		val = extclk->get_clock();
		break;
	}
	return val;
}

void MediaDecoder::saveFrame(AVFrame *pFrame, int width, int height,
		int iFrame) {
	FILE *pFile;
	char szFilename[32];
	int y;

	sprintf(szFilename, "/sdcard/frame%d.ppm", iFrame);
	pFile = fopen(szFilename, "wb");
	if (pFile == NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for (y = 0; y < height; y++) {
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
	}

	// Close file
	fclose(pFile);
}

MediaDecoder::MediaDecoder() {
	memset(pictq, 0, sizeof(pictq));
	pictq_size = pictq_rindex = pictq_windex = 0;

//	decodedFormat = PIX_FMT_YUV420P;
	decodedFormat = PIX_FMT_RGB24;

	img_convert_ctx = NULL;

	fileEof = false;
	videoDecodeEnd = false;

	pFormatCtx = avformat_alloc_context();
	assert(pFormatCtx);

	memset(st_index, -1, sizeof(st_index));
	packet_queue_init(&videoq);

	videoSrcFrame = av_frame_alloc();

	decoedPictureNum = 0;

	callbacker = NULL;

	pktReadEnd = 0;

	frame_last_dropped_pts = frame_last_pts = AV_NOPTS_VALUE;

	frame_last_duration = 0;

	frame_drops_late = frame_drops_early = 0;

	video_current_pos = -1;

	frame_timer = 0;

	//audio
	swr_ctx = NULL;
	audioStream = NULL;
	audio_buf = audio_buf1 = NULL;
	memset(silence_buf, 0, sizeof(silence_buf));
	audio_buf1_size = audio_write_buf_size = audio_buf_size = audio_buf_index =
			audio_hw_buf_size = 0;

	if (frame_queue_init(&sampq, &audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
		LOGE("frame_queue_init error");
	packet_queue_init(&audioq);

	max_frame_duration = 10;

	lastPaused = paused = false;
	step = false;
}

MediaDecoder::~MediaDecoder() {
	if (pFormatCtx != NULL) {
		avformat_close_input(&pFormatCtx);
	}

	packet_queue_destroy(&videoq);

	if (img_convert_ctx) {
		sws_freeContext(img_convert_ctx);
	}

	if (videoSrcFrame) {
		av_frame_free(&videoSrcFrame);
	}

	int i;
	for (i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; i++) {
		free_picture(&pictq[i]);
	}

	//audio
	packet_queue_abort(&audioq);
	frame_queue_destory(&sampq);
	packet_queue_destroy(&audioq);
}

void MediaDecoder::setFrameQueueLocker(SDL_mutex *frameQueueMutex,
		SDL_cond *frameQueueCond) {
	frame_queue_set_locker(&sampq, frameQueueMutex, frameQueueCond);
}

void MediaDecoder::setDecodeInterruptCb(int (*cb)(void *), void *cbParam) {
	pFormatCtx->interrupt_callback.callback = cb;
	pFormatCtx->interrupt_callback.opaque = cbParam;
}

int MediaDecoder::synchronize_audio(int nb_samples) {
	int wanted_nb_samples = nb_samples;

	return wanted_nb_samples;
}

int MediaDecoder::audio_decode_frame() {
	MediaDecoder *audioDecoder = this;
	int data_size, resampled_data_size = -1;
	int64_t dec_channel_layout;
	int wanted_nb_samples;
	Frame *af;

    if (paused)
        return -1;

	do {
		if (!(af = frame_queue_peek_readable(&audioDecoder->sampq))) {
			LOGE("decode_thread, %d, frame_queue_push!!", __LINE__);
			return -1;
		}
		if(af->serial != audioDecoder->audioq.serial)
		    frame_queue_next(&audioDecoder->sampq);
		else
		    break;
	} while (1);

	{
		data_size = av_samples_get_buffer_size(NULL, af->mtAudioFrame.channels,
				af->mtAudioFrame.nb_samples,
				(AVSampleFormat) (af->mtAudioFrame.format), 1);

		dec_channel_layout =
				(af->mtAudioFrame.channel_layout
						&& af->mtAudioFrame.channels
								== av_get_channel_layout_nb_channels(
										af->mtAudioFrame.channel_layout)) ?
						af->mtAudioFrame.channel_layout :
						av_get_default_channel_layout(
								af->mtAudioFrame.channels);
		wanted_nb_samples = synchronize_audio(af->mtAudioFrame.nb_samples);

		if (af->mtAudioFrame.format != audioDecoder->audio_src.fmt
				|| dec_channel_layout != audioDecoder->audio_src.channel_layout
				|| af->mtAudioFrame.sample_rate != audioDecoder->audio_src.freq
				|| (wanted_nb_samples != af->mtAudioFrame.nb_samples
						&& !audioDecoder->swr_ctx)) {

			swr_free(&audioDecoder->swr_ctx);
			audioDecoder->swr_ctx = swr_alloc_set_opts(NULL,
					audioDecoder->audio_tgt.channel_layout,
					audioDecoder->audio_tgt.fmt, audioDecoder->audio_tgt.freq,
					dec_channel_layout,
					(AVSampleFormat) af->mtAudioFrame.format,
					af->mtAudioFrame.sample_rate, 0, NULL);
			if (!audioDecoder->swr_ctx || swr_init(audioDecoder->swr_ctx) < 0) {
				av_log(NULL, AV_LOG_ERROR,
						"Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
						af->mtAudioFrame.sample_rate,
						av_get_sample_fmt_name(
								(AVSampleFormat) af->mtAudioFrame.format),
						af->mtAudioFrame.channels, audioDecoder->audio_tgt.freq,
						av_get_sample_fmt_name(audioDecoder->audio_tgt.fmt),
						audioDecoder->audio_tgt.channels);
				swr_free(&audioDecoder->swr_ctx);
				goto fail;
			}
			audioDecoder->audio_src.channel_layout = dec_channel_layout;
			audioDecoder->audio_src.channels = af->mtAudioFrame.channels;
			audioDecoder->audio_src.freq = af->mtAudioFrame.sample_rate;
			audioDecoder->audio_src.fmt =
					(AVSampleFormat) af->mtAudioFrame.format;
		}

		if (audioDecoder->swr_ctx) {
			const uint8_t **in = (const uint8_t **) af->mtAudioFrame.data;
			uint8_t **out = &audioDecoder->audio_buf1;
			int out_count = (int) ((int64_t) wanted_nb_samples
					* audioDecoder->audio_tgt.freq
					/ af->mtAudioFrame.sample_rate + 256);
			int out_size = av_samples_get_buffer_size(NULL,
					audioDecoder->audio_tgt.channels, out_count,
					audioDecoder->audio_tgt.fmt, 0);
			int len2;
			if (out_size < 0) {
				av_log(NULL, AV_LOG_ERROR,
						"av_samples_get_buffer_size() failed\n");
				goto fail;
			}

			if (wanted_nb_samples != af->mtAudioFrame.nb_samples) {
				if (swr_set_compensation(audioDecoder->swr_ctx,
						(wanted_nb_samples - af->mtAudioFrame.nb_samples)
								* audioDecoder->audio_tgt.freq
								/ af->mtAudioFrame.sample_rate,
						wanted_nb_samples * audioDecoder->audio_tgt.freq
								/ af->mtAudioFrame.sample_rate) < 0) {
					av_log(NULL, AV_LOG_ERROR,
							"swr_set_compensation() failed\n");
					goto fail;
				}
			}

			av_fast_malloc(&audioDecoder->audio_buf1,
					&audioDecoder->audio_buf1_size, out_size);
			if (!audioDecoder->audio_buf1) {
				LOGE("audio_decode_frame, %d, %p, want size = %d, return %u",
						__LINE__, audioDecoder->audio_buf1, out_size,
						audioDecoder->audio_buf1_size);
				goto fail;
			}

			len2 = swr_convert(audioDecoder->swr_ctx, out, out_count, in,
					af->mtAudioFrame.nb_samples);
			if (len2 < 0) {
				LOGE("audio_decode_frame, %d", __LINE__);
				av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
				goto fail;
			}
			if (len2 == out_count) {
				av_log(NULL, AV_LOG_WARNING,
						"audio buffer is probably too small\n");
				if (swr_init(audioDecoder->swr_ctx) < 0)
					swr_free(&audioDecoder->swr_ctx);
			}
			audioDecoder->audio_buf = audioDecoder->audio_buf1;
			resampled_data_size = len2 * audioDecoder->audio_tgt.channels
					* av_get_bytes_per_sample(audioDecoder->audio_tgt.fmt);
		} else {
			audioDecoder->audio_buf = af->mtAudioFrame.data[0];
			resampled_data_size = data_size;
		}

		audioDecoder->audio_clock_serial = af->serial;
#ifdef FFP_SHOW_AUDIO_DELAY
#ifdef DEBUG
		{
			static double last_clock;
			printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
					is->audio_clock - last_clock,
					is->audio_clock, audio_clock0);
			last_clock = is->audio_clock;
		}
#endif
#endif
	}
	fail: frame_queue_next(&audioDecoder->sampq);
	return resampled_data_size;
}

int MediaDecoder::stream_component_open(int streamIndex) {
	AVCodecContext *avctx;
	AVCodec *codec;

	int sample_rate, nb_channels;
	int64_t channel_layout;
	int ret;

	if (streamIndex < 0 || streamIndex >= pFormatCtx->nb_streams) {
		LOGE("stream_component_open failed: streamIndex = %d", streamIndex);
		return -1;
	}
	avctx = pFormatCtx->streams[streamIndex]->codec;

	codec = avcodec_find_decoder(avctx->codec_id);
	if (!codec) {
		av_log(NULL, AV_LOG_WARNING, "No codec could be found with id %d\n",
				avctx->codec_id);
		return -1;
	}

	avctx->codec_id = codec->id;
	ret = avcodec_open2(avctx, codec, NULL);
	if (ret < 0) {
	    char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
		LOGE("avcodec_open2 failed, err = %s", av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret));
		return -1;
	}

	switch (avctx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		videoStream = pFormatCtx->streams[streamIndex];
		packet_queue_start(&videoq);
		break;
	case AVMEDIA_TYPE_AUDIO:
		sample_rate = avctx->sample_rate;
		nb_channels = avctx->channels;
		channel_layout = avctx->channel_layout;

		audio_hw_buf_size = ret;
		audio_src = audio_tgt;
		audio_buf_size = 0;
		audio_buf_index = 0;
		audio_stream = streamIndex;
		audioStream = pFormatCtx->streams[streamIndex];
		packet_queue_start(&audioq);
		memset(&auddec, 0, sizeof(Decoder));
		auddec.avctx = avctx;
		auddec.queue = &audioq;
		auddec.empty_queue_cond = continue_read_thread;
		auddec.start_pts = AV_NOPTS_VALUE;

		break;
	default:
		break;
	}
	return 0;
}

void MediaDecoder::stream_component_close(int stream_index) {
	AVCodecContext *avctx;

	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams)
		return;
	avctx = pFormatCtx->streams[stream_index]->codec;

	switch (avctx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		packet_queue_flush(&videoq);
		break;
	case AVMEDIA_TYPE_AUDIO:
		packet_queue_flush(&audioq);
		break;
	default:
		break;
	}

	pFormatCtx->streams[stream_index]->discard = AVDISCARD_ALL;
	avcodec_flush_buffers(avctx); //可能有问题
	avcodec_close(avctx);
	switch (avctx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		videoStream = NULL;
		st_index[stream_index] = -1;
		break;
	case AVMEDIA_TYPE_AUDIO:
		audioStream = NULL;
		st_index[stream_index] = -1;
		break;
	default:
		break;
	}
}

int MediaDecoder::open(const MediaFileInfo &mfi) {
	if (mfi.fileName.empty()) {
		return -1;
	}

	fileInfo = mfi;
	const char* fName = fileInfo.fileName.c_str();

	fileInfo.vidclk = new MediaClock(&videoq.serial);
	fileInfo.extclk = new MediaClock(NULL);
	fileInfo.audclk = new MediaClock(NULL);

	int ret = avformat_open_input(&pFormatCtx, fName, NULL, NULL);
	if (ret != 0) {
	    char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
		LOGE("avformat_open_input failed, file = %s, err = %s",
				fName, av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret));
		return -1;
	}

	ret = avformat_find_stream_info(pFormatCtx, NULL);
	if (ret < 0) {
	    char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
		LOGE("avformat_find_stream_info failed, err = %s", av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret));
		return -1;
	}

	if (pFormatCtx->pb) {
		// FIXME hack, ffplay maybe should not use url_feof() to test for the end
		pFormatCtx->pb->eof_reached = 0;
	}

	if (fileInfo.seekByBytes < 0) {
		fileInfo.seekByBytes = !!(pFormatCtx->iformat->flags & AVFMT_TS_DISCONT)
				&& strcmp("ogg", pFormatCtx->iformat->name);
	}

	max_frame_duration =
			(pFormatCtx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

	int i;
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			st_index[AVMEDIA_TYPE_VIDEO] = i;
			break;
		}
	}

	//audio
	st_index[AVMEDIA_TYPE_AUDIO] =
        av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO,
                            -1,
                            st_index[AVMEDIA_TYPE_VIDEO],
                            NULL, 0);
    if(st_index[AVMEDIA_TYPE_AUDIO] < 0) {
        LOGE("%s: could not find audio stream\n", fName);
        //return -1;
    }

    av_dump_format(pFormatCtx, 0, fName, 0);

    if(hasAudio()) {
        //packet_queue_start(&audioq);
    	//audioStream = pFormatCtx->streams[st_index[AVMEDIA_TYPE_AUDIO]];
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }

//	if (!hasVideo()) {
//		LOGE("Didn't find a video stream.");
//		av_dump_format(pFormatCtx, 0, fName, 0);
//		return -1;
//	} else {
	if (hasVideo()) {
		//packet_queue_start(&videoq);
		//videoStream = pFormatCtx->streams[st_index[AVMEDIA_TYPE_VIDEO]];
		stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
	}

	return 0;
}

int MediaDecoder::packetQueuAbort() {
	packet_queue_abort(&videoq);
	packet_queue_abort(&audioq);
}

int MediaDecoder::close() {
	if (hasVideo()) {
		stream_component_close(st_index[AVMEDIA_TYPE_VIDEO]);
	}
	if (hasAudio()) {
		stream_component_close(st_index[AVMEDIA_TYPE_AUDIO]);
	}
	avformat_close_input(&pFormatCtx);

	frame_queue_signal(&sampq);

	return 0;
}

ReadPktStatus MediaDecoder::readPkt() {/* if the queue are full, no need to read more */
	if (st_index[AVMEDIA_TYPE_VIDEO] > 0 && pktReadEnd == videoq.serial) {
		return READ_PKT_END;
	}

	if (st_index[AVMEDIA_TYPE_VIDEO] > 0
			&& (videoq.size > MAX_QUEUE_SIZE
					|| (videoq.nb_packets > MIN_FRAMES || videoq.abort_request))) {
//		LOGE("%p not read pkt", this);
//		LOGE("xxx no read pkt for : %d, %d, %d, %d",
//				videoq.size > MAX_QUEUE_SIZE, videoq.nb_packets > MIN_FRAMES,
//				st_index[AVMEDIA_TYPE_VIDEO] < 0, videoq.abort_request);
		return READ_PKT_QUEUE_FULL;
	}

	if (fileEof) {
		if (hasVideo()) {
			packet_queue_put_nullpacket(&videoq, st_index[AVMEDIA_TYPE_VIDEO]);
		}
		if (hasAudio()) {
			packet_queue_put_nullpacket(&audioq, st_index[AVMEDIA_TYPE_AUDIO]);
		}
		fileEof = false;
		return READ_PKT_EOF;
	}

	AVPacket pkt1, *pkt = &pkt1;
	int ret = av_read_frame(pFormatCtx, pkt);
	if (ret < 0) {
		if ((ret == AVERROR_EOF) || url_feof(pFormatCtx->pb)) {
			fileEof = true;

			if (pFormatCtx->pb && pFormatCtx->pb->error) {
				return REAK_PKT_ERROR;
			}

			return READ_PKT_EOF;
		}

		if (pFormatCtx->pb && pFormatCtx->pb->error) {
			pktReadEnd = videoq.serial;
			return READ_PKT_END;
		}

		return REAK_PKT_ERROR;
	}

	if (pkt->stream_index == st_index[AVMEDIA_TYPE_VIDEO]) {
		packet_queue_put(&videoq, pkt);
	} else if (pkt->stream_index == st_index[AVMEDIA_TYPE_AUDIO]) {
		packet_queue_put(&audioq, pkt);
	} else {
		av_free_packet(pkt);
	}

	return READ_PKT_OK;
}

static int decoder_decode_frame(MediaDecoder *audioDecoder, Decoder *d,
		AVFrame *frame, AVSubtitle *sub) {
	int got_frame = 0;

	d->flushed = 0;

	do {
		int ret = -1;

		if (d->queue->abort_request)
			return -1;

		if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
			AVPacket pkt;
			do {
				if (d->queue->nb_packets == 0)
					SDL_CondSignal(d->empty_queue_cond);
				if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
					return -1;
				if (is_flush_pkt(&pkt)) {
					avcodec_flush_buffers(d->avctx);
					d->finished = 0;
					d->flushed = 1;
					d->next_pts = d->start_pts;
					d->next_pts_tb = d->start_pts_tb;
				}
			} while (is_flush_pkt(&pkt) || d->queue->serial != d->pkt_serial);
			av_free_packet(&d->pkt);
			d->pkt_temp = d->pkt = pkt;
			d->packet_pending = 1;
		}

		switch (d->avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			ret = avcodec_decode_audio4(d->avctx, frame, &got_frame,
					&d->pkt_temp);
			if (got_frame) {
				AVRational tb = (AVRational ) { 1, frame->sample_rate };
				if (frame->pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(frame->pts, d->avctx->time_base,
							tb);
				else if (frame->pkt_pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(frame->pkt_pts,
							av_codec_get_pkt_timebase(d->avctx), tb);
				else if (d->next_pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
				if (frame->pts != AV_NOPTS_VALUE) {
					d->next_pts = frame->pts + frame->nb_samples;
					d->next_pts_tb = tb;
				}
			}
			break;
			// FFP_MERGE: case AVMEDIA_TYPE_SUBTITLE:
		default:
			break;
		}

		if (ret < 0) {
			d->packet_pending = 0;
		} else {
			d->pkt_temp.dts = d->pkt_temp.pts = AV_NOPTS_VALUE;
			if (d->pkt_temp.data) {
				if (d->avctx->codec_type != AVMEDIA_TYPE_AUDIO)
					ret = d->pkt_temp.size;
				d->pkt_temp.data += ret;
				d->pkt_temp.size -= ret;
				if (d->pkt_temp.size <= 0)
					d->packet_pending = 0;
			} else {
				if (!got_frame) {
					d->packet_pending = 0;
					d->finished = d->pkt_serial;
				}
			}
		}
	} while (!got_frame && !d->finished);

	return got_frame;
}

DecodePktStatus MediaDecoder::decodeAudio() {
	MediaDecoder *audioDecoder = (MediaDecoder*) this;
	AVFrame *frame = av_frame_alloc();
	Frame *af;
	int got_frame = 0;
	AVRational tb;
	int ret = 0;
	DecodePktStatus status = DECODE_PKT_OK;

	if (!frame)
		return DECODE_PKT_ERROR;		//AVERROR(ENOMEM);

	do {
		//if full goto the_end, for other track to continue
		if (frame_queue_is_full(&audioDecoder->sampq)) {
			status = DECODE_PKT_QUEUE_FULL;
			goto the_end;
		}

		if ((got_frame = decoder_decode_frame(audioDecoder,
				&audioDecoder->auddec, frame,
				NULL)) < 0) {
			status = DECODE_PKT_ERROR;
			goto the_end;
		}

		if (got_frame) {
			tb = (AVRational ) { 1, frame->sample_rate };

			if (!(af = frame_queue_peek_writable(&audioDecoder->sampq)))
				goto the_end;

			af->pts = (frame->pts == AV_NOPTS_VALUE) ?
			NAN :
														frame->pts * av_q2d(tb);
			af->pos = av_frame_get_pkt_pos(frame);
			af->serial = audioDecoder->auddec.pkt_serial;
			af->duration =
					av_q2d(
							(AVRational ) { frame->nb_samples,
											frame->sample_rate });

			af->mtAudioFrame.copyFromAvFrame(frame);
			frame_queue_push(&audioDecoder->sampq);
			break;
		}
	} while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);

	the_end: av_frame_free(&frame);
	return status;
}

bool MediaDecoder::get_video_frame(AVFrame *frame, AVPacket *pkt, int serial) {
	int got_picture;

	int ret = avcodec_decode_video2(videoStream->codec, frame, &got_picture,
			pkt);
	if (ret < 0) {
	    char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
		LOGE("avcodec_decode_video2 failed:%s", av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret));
		return 0;
	}

	if (!got_picture && !pkt->data) {
		pktReadEnd = serial;
	}

	if (got_picture) {
		double dpts = NAN;

		frame->pts = av_frame_get_best_effort_timestamp(frame);

		if (frame->pts != AV_NOPTS_VALUE)
			dpts = av_q2d(videoStream->time_base) * frame->pts;

		frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(pFormatCtx,
				videoStream, frame);

		if (fileInfo.frameDrop) {
			lock();
			if (frame_last_pts != AV_NOPTS_VALUE && frame->pts != AV_NOPTS_VALUE) {
				//视频时钟与外部时钟的差值
				double clockdiff = fileInfo.vidclk->get_clock()
						- fileInfo.getMasterClock();
				//当前帧与上一帧的时钟差值
				double ptsdiff = dpts - frame_last_pts;
				//clockdiff + ptsdiff判断当前帧与前帧的时间间隔是否大于落后时钟的值，
				//落后则丢掉此帧并通过return 0再次调用get_video_frame()取新的帧
				if (!isnan(clockdiff) && fabs(clockdiff) < AV_NOSYNC_THRESHOLD
						&& !isnan(ptsdiff) && ptsdiff > 0
						&& clockdiff + ptsdiff < 0
						&& ptsdiff < AV_NOSYNC_THRESHOLD && videoq.nb_packets) {
					frame_last_dropped_pos = av_frame_get_pkt_pos(frame);
					frame_last_dropped_pts = dpts;
					frame_last_dropped_serial = serial;
					frame_drops_early++;
					av_frame_unref(frame);
					ret = 0;
				}
			}
			unlock();
		}

		return 1;
	} else {
		//LOGE("avcodec_decode_video2 no get picture, try again");
	}
	return 0;
}

void MediaDecoder::free_picture(VideoPicture *vp) {
	if (vp->decodedFrame) {
		av_frame_free(&vp->decodedFrame);
		vp->decodedFrame = NULL;
	}

	if (vp->frameBuffer) {
		av_free(vp->frameBuffer);
		vp->frameBuffer = NULL;
	}
}

/* allocate a picture (needs to do that in main thread to avoid
 potential locking problems */
void MediaDecoder::alloc_picture(VideoPicture *vp) {
	int64_t bufferdiff;

	free_picture(vp);

	vp->decodedFrame = av_frame_alloc();
	if (!vp->decodedFrame) {
		av_log(NULL, AV_LOG_FATAL, "av_frame_alloc error\n");
		assert(0);
	}

	LOGD("alloc piccture, vp->w = %d, h = %d", vp->width, vp->height);
	vp->frameBuffer = (uint8_t *) av_malloc(
			avpicture_get_size(decodedFormat, vp->width, vp->height));
	avpicture_fill((AVPicture *) vp->decodedFrame, vp->frameBuffer,
			decodedFormat, vp->width, vp->height);

	lock();
	vp->allocated = 1;
	unlock();
}

bool MediaDecoder::isPictqFull() {
	lock();
	bool ret = pictq_size >= VIDEO_PICTURE_QUEUE_SIZE - 1;
	unlock();
	return ret;
}

bool MediaDecoder::isPictqEmpty() {
	lock();
	bool ret = pictq_size < 1;
	unlock();
	return ret;
}

int MediaDecoder::queue_picture(AVFrame *src_frame, double pts, int64_t pos,
		int serial) {
	VideoPicture *vp;

	if (videoq.abort_request) {
		return -1;
	}

	vp = &pictq[pictq_windex];

	vp->sar = src_frame->sample_aspect_ratio;

	/* alloc or resize hardware picture buffer */
	if (!vp->decodedFrame) {
		vp->width = src_frame->width;
		vp->height = src_frame->height;
		alloc_picture(vp);

		if (videoq.abort_request) {
			return -1;
		}
	}

	/* if the frame is not skipped, then display it */
	if (vp->decodedFrame) {
		if (img_convert_ctx == NULL) {
			LOGD(
					"sws_getContext : src->w, h =%d,%d  format = %d, target->w,h=%d,%d",
					vp->width, vp->height, src_frame->format, vp->width,
					vp->height);
			img_convert_ctx = sws_getContext(vp->width, vp->height,
					(AVPixelFormat) src_frame->format, vp->width, vp->height,
					decodedFormat,
					SWS_BICUBIC, NULL, NULL, NULL);
			if (img_convert_ctx == NULL) {
				av_log(NULL, AV_LOG_FATAL, "sws_getCachedContext failed");
				exit(1);
			}
		}
		int ret = sws_scale(img_convert_ctx, src_frame->data,
				src_frame->linesize, 0, vp->height, vp->decodedFrame->data,
				vp->decodedFrame->linesize);
		if (ret < 0) {
			LOGE("sws_scale failed, ret = %d", ret);
			assert(0);
		}
//		saveFrame(vp->decodedFrame, vp->width, vp->height, decoedPictureNum);
#if 0
		AVCodecContext *avctx = pFormatCtx->streams[AVMEDIA_TYPE_VIDEO]->codec;
		AVFrame *pFrameRGB = av_frame_alloc();
		assert(pFrameRGB);
		uint8_t *buffer;
		int numBytes = avpicture_get_size(PIX_FMT_RGB24, avctx->width,
				avctx->height);
		buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
		avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24,
				vp->width, vp->height);
		static struct SwsContext *pSwsCtx = sws_getContext(avctx->width,
				avctx->height, (AVPixelFormat) src_frame->format, avctx->width,
				avctx->height, PIX_FMT_RGB24,
				SWS_BICUBIC, NULL, NULL, NULL);
		ret = sws_scale(pSwsCtx, src_frame->data, src_frame->linesize, 0,
				avctx->height, pFrameRGB->data, pFrameRGB->linesize);
		assert(ret >= 0);
		saveFrame(pFrameRGB, vp->width, vp->height, decoedPictureNum + 100);
#endif
		decoedPictureNum++;

		vp->pts = pts;
		vp->pos = pos;
		vp->serial = serial;
		vp->index = decoedPictureNum;

		/* now we can update the picture count */
		if (++pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
			pictq_windex = 0;
		lock();
		pictq_size++;
		unlock();
	}
	return 0;
}

DecodePktStatus MediaDecoder::decodeVideo() {
	int serial = 0;

	if (videoDecodeEnd || videoq.abort_request) {
		return DECODE_PKT_END;
	}

	if (isPictqFull()) {
		return DECODE_PKT_QUEUE_FULL;
	}

	DecodePktStatus status = DECODE_PKT_OK;

	AVPacket pkt = { 0 };

	while (true) {
	    av_frame_unref(videoSrcFrame);
		av_free_packet(&pkt);
		int ret = packet_queue_get(&videoq, &pkt, 0, &serial);
		if (ret < 0) {
			LOGE("packet_queue_get failed: abort request");
			videoDecodeEnd = true;
			status = DECODE_PKT_END;
			goto END;
		}
		if (ret == 0) {
			//LOGE("packet_queue_get failed: no pkt");
			status = DECODE_PKT_PKTQ_EMPTY;
			goto END;
		}

		if (!get_video_frame(videoSrcFrame, &pkt, serial)) {
			//LOGE("get_video_frame failed, try again");
			continue;
		}

		double pts =
				(videoSrcFrame->pts == AV_NOPTS_VALUE) ?
						NAN :
						videoSrcFrame->pts * av_q2d(videoStream->time_base);

		ret = queue_picture(videoSrcFrame, pts,
				av_frame_get_pkt_pos(videoSrcFrame), serial);

		av_frame_unref(videoSrcFrame);
		if (ret < 0) {
			LOGE("decodeVideo error 2");
			videoDecodeEnd = true;
			status = DECODE_PKT_END;
			goto END;
		}

		break;
	}
	END: av_free_packet(&pkt);

	return status;
}

int MediaDecoder::getAudioDecodedData() {
	return 0;
}

void MediaDecoder::pictq_next_picture() {
	/* update queue size and signal for next picture */
	if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
		pictq_rindex = 0;
	lock();
	pictq_size--;
	unlock();
}

void MediaDecoder::update_video_pts(double pts, int64_t pos, int serial) {
	/* update current video pts */
	fileInfo.vidclk->set_clock(pts, serial);
	fileInfo.extclk->sync_clock_to(fileInfo.vidclk);
	video_current_pos = pos;
	frame_last_pts = pts;
}

double MediaDecoder::compute_target_delay(double delay) {
	double sync_threshold, diff;

	/* update delay to follow master synchronisation source */
	if (fileInfo.avSyncType != AV_SYNC_VIDEO_MASTER) {
		/* if video is slave, we try to correct big delays by
		 duplicating or deleting a frame */
		diff = fileInfo.vidclk->get_clock() - fileInfo.getMasterClock();

		/* skip or repeat frame. We take into account the
		 delay to compute the threshold. I still don't know
		 if it is the best guess */
		//sync_threshold
		//sync_threshold决定了要不要采用同步纠正，落在(-sync_threshold, sync_threshold)间的不需要同步纠正
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN,
				FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < max_frame_duration) {
			if (diff <= -sync_threshold) {
				//视频播放太慢了
				delay = FFMAX(0, delay + diff);
			} else if (diff
					>= sync_threshold&& delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
				//视频播太快，且当前帧间隔上一帧太长了，让上一帧重复播放，并delay到当前帧时间到了
				delay = delay + diff;
			} else if (diff >= sync_threshold) {
				//视频播太快，且当前帧间隔上一帧太长了，让上一帧重复播放，并延长delay
				delay = 2 * delay;
			}
		}
	}

	av_dlog(NULL, "video: delay=%0.3f A-V=%f\n", delay, -diff);

	return delay;
}

VideoPicture *MediaDecoder::choiceNextVideoPicture(int *remaingTimes) {
	double time;

	if (videoStream) {
		int redisplay = 0;
		retry: if (pictq_size == 0) {
			lock();
			//如果有早期丢帧，用丢弃帧的pts更新视频时钟
			if (frame_last_dropped_pts != AV_NOPTS_VALUE
					&& frame_last_dropped_pts > frame_last_pts) {
				update_video_pts(frame_last_dropped_pts, frame_last_dropped_pos,
						frame_last_dropped_serial);
				frame_last_dropped_pts = AV_NOPTS_VALUE;
			}
			unlock();
			// nothing to do, no picture to display in the queue
			LOGE(
					"choiceNextVideoPicture()：nothing to do, no picture to display in the queue");
			return NULL;
		}

		double last_duration, duration, delay;
		/* dequeue the picture */
		VideoPicture *vp = &pictq[pictq_rindex];

		if (vp->serial != videoq.serial) {
			pictq_next_picture();
			redisplay = 0;
			goto retry;
		}

		if (paused) {
			goto display;
		}

		/* compute nominal last_duration */
		last_duration = vp->pts - frame_last_pts;
		if (!isnan(last_duration) && last_duration > 0
				&& last_duration < max_frame_duration) {
			/* if duration of the last frame was sane, update last_duration in video state */
			frame_last_duration = last_duration;
		}
		if (redisplay)
			delay = 0.0;
		else
			delay = compute_target_delay(frame_last_duration);

		time = av_gettime() / 1000000.0;
		if (time < frame_timer + delay && !redisplay) {
			//当前时间还没到下一帧播放时间，并且不必重播上一帧，则直接返回
			*remaingTimes = FFMIN(frame_timer + delay - time, *remaingTimes);
			//还不到播放时间
			return NULL;
		}

		frame_timer += delay;
		if (delay > 0 && time - frame_timer > AV_SYNC_THRESHOLD_MAX) {
			//画面与时钟已经失去同步了，通过更新frame_timer来进入后面的丢帧操作
			frame_timer = time;
		}

		lock();
		if (!redisplay && !isnan(vp->pts)) {
			update_video_pts(vp->pts, vp->pos, vp->serial);
		}
		unlock();

		if (pictq_size > 1) {
			VideoPicture *nextvp = &pictq[(pictq_rindex + 1)
					% VIDEO_PICTURE_QUEUE_SIZE];
			duration = nextvp->pts - vp->pts;
			if (!step && (redisplay || fileInfo.frameDrop)
					&& time > frame_timer + duration) {
				//redisplay判断帧有没丢过
				if (!redisplay) {
					frame_drops_late++;
				}
				pictq_next_picture();
				redisplay = 0;
				goto retry;
			}
		}

		display: pictq_next_picture();

		if (step && !paused) {
			stream_toggle_pause();
		}

		return vp;
	}

	return NULL;
}

//返回匹配当前时钟的videopicture
VideoPicture *MediaDecoder::getNextFrame(int *remaingTimes) {
	if (isPictqEmpty()) {
		return NULL;
	}

	VideoPicture *vp = choiceNextVideoPicture(remaingTimes);

	return vp;
}

int MediaDecoder::getTimeLength() {
	return timeLength;
}

bool MediaDecoder::hasVideo() {
	return st_index[AVMEDIA_TYPE_VIDEO] > -1;
}

bool MediaDecoder::hasAudio() {
	return st_index[AVMEDIA_TYPE_AUDIO] > -1;
}

void MediaDecoder::setPause() {
	if (paused != lastPaused) {
		lastPaused = paused;
		if (paused) {
			read_pause_return = av_read_pause(pFormatCtx);
		} else {
			av_read_play(pFormatCtx);
		}
	}
}

bool MediaDecoder::isPaused() {
	return paused;
}
/* pause or resume the video */
void MediaDecoder::stream_toggle_pause() {
	if (paused) {
		frame_timer += av_gettime() / 1000000.0 + fileInfo.vidclk->pts_drift
				- fileInfo.vidclk->pts;
		if (read_pause_return != AVERROR(ENOSYS)) {
			fileInfo.vidclk->paused = 0;
		}
		fileInfo.vidclk->set_clock(fileInfo.vidclk->get_clock(),
				fileInfo.vidclk->serial);
	}
	fileInfo.extclk->set_clock(fileInfo.extclk->get_clock(),
			fileInfo.extclk->serial);
	paused = fileInfo.audclk->paused = fileInfo.vidclk->paused =
			fileInfo.extclk->paused = !paused;
}

void MediaDecoder::togglePause() {
	stream_toggle_pause();
	step = false;
}

int MediaDecoder::seekTo(int value, bool seekByTime, bool isPlaying) {
	int64_t seek_pos, seek_rel = 0;

	if (seekByTime && (pFormatCtx->duration <= 0)) {
		LOGE("seekTo failed: seekByTime but pFormatCtx->duration <= 0");
		return -1;
	}

	if (fileInfo.seekByBytes || pFormatCtx->duration <= 0) {
		uint64_t size = avio_size(pFormatCtx->pb);
		seek_pos = size * value / 100;
	} else {
		double frac;
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		tns = pFormatCtx->duration / 1000000LL;
		frac = (seekByTime ? ((double) value / (tns * 1000)) : value / 100.0f);
		thh = tns / 3600;
		tmm = (tns % 3600) / 60;
		tss = (tns % 60);
		ns = frac * tns;
		hh = ns / 3600;
		mm = (ns % 3600) / 60;
		ss = (ns % 60);
		av_log(NULL, AV_LOG_INFO,
				"Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n",
				frac * 100, hh, mm, ss, thh, tmm, tss);
		seek_pos = pFormatCtx->duration * frac;
		if (pFormatCtx->start_time != AV_NOPTS_VALUE) {
			seek_pos += pFormatCtx->start_time;
		}
	}

	int64_t seek_target = seek_pos;
	int64_t seek_min = seek_rel > 0 ? seek_target - seek_rel + 2 : INT64_MIN;
	int64_t seek_max = seek_rel < 0 ? seek_target - seek_rel - 2 : INT64_MAX;
	// FIXME the +-2 is due to rounding being not done in the correct direction in generation
	//      of the seek_pos/seek_rel variables
	seekTo(seek_target, seek_min, seek_max, isPlaying);
}

int MediaDecoder::seekTo(int64_t tragetPos, int64_t minPos, int64_t maxPos,
		bool isPlaying) {
	int seek_flags = 0;
	if (fileInfo.seekByBytes || pFormatCtx->duration <= 0) {
		seek_flags |= AVSEEK_FLAG_BYTE;
	}

	int ret = avformat_seek_file(pFormatCtx, -1, minPos, tragetPos, maxPos,
			seek_flags);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "%s: error while seeking\n",
				fileInfo.fileName.c_str());
	} else if (isPlaying) {
		if (hasVideo()) {
			packet_queue_flush(&videoq);
			packet_queue_put_flush_pkt(&videoq);
		}
		if (hasAudio()) {
            packet_queue_flush(&audioq);
            packet_queue_put_flush_pkt(&audioq);
        }
		if (seek_flags & AVSEEK_FLAG_BYTE) {
			fileInfo.extclk->set_clock(NAN, 0);
		} else {
			fileInfo.extclk->set_clock(tragetPos / (double) AV_TIME_BASE, 0);
		}
	}

	if (isPlaying) {
		if (paused) {
			stream_toggle_pause();
			step = true;
		}
		fileEof = false;
	}

	return ret;
}

int MediaDecoder::getPlayedTime() {
	return fileInfo.getMasterClock() * 1000;
}
