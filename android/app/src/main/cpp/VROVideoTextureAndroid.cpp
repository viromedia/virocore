//
//  VROVideoTextureAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateOpenGL.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "VROVideoTextureAndroid.h"
#include "VROFrameSynchronizer.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// pre-recorded sound clips, both are 8 kHz mono 16-bit signed little endian
static const char hello[] =
#include "hello_clip.h"
;

enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDecodeDone,
    kMsgSeek,
};

VROVideoTextureAndroid::VROVideoTextureAndroid() :
        VROVideoTexture(VROTextureType::TextureEGLImage),
        _looper(nullptr),
        _paused(true),
        _loop(true) {

    _mediaData.fd = -1;
    _mediaData.window = nullptr;
    _mediaData.extractor = nullptr;
    _mediaData.videoCodec = nullptr;
    _mediaData.videoCodecTrack = -1;
    _mediaData.audioCodec = nullptr;
    _mediaData.audioCodecTrack = -1;
    _mediaData.renderStart = 0;
    _mediaData.sawInputEOS = false;
    _mediaData.sawOutputEOS = false;
    _mediaData.isPlaying = false;
    _mediaData.renderOnce = false;
}

VROVideoTextureAndroid::~VROVideoTextureAndroid() {
    delete (_looper);
}

void VROVideoTextureAndroid::createVideoTexture() {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);

    // Can't do mipmapping with video textures, and clamp to edge is only option
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, textureId, true));
    setSubstrate(std::move(substrate));

    JNIEnv *env = VROPlatformGetJNIEnv();
    jobject jsurface = VROPlatformCreateVideoSink(textureId);

    _mediaData.window = ANativeWindow_fromSurface(env, jsurface);
}

void VROVideoTextureAndroid::loadVideo(std::string url,
                                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                       VRODriver &driver) {

    frameSynchronizer->addFrameListener(shared_from_this());
    createVideoTexture();

    JNIEnv *env = VROPlatformGetJNIEnv();
    AAssetManager *assetMgr = VROPlatformGetAssetManager();

    off_t outStart, outLen;
    int fd = AAsset_openFileDescriptor(AAssetManager_open(assetMgr, "testfile.mp4", 0),
                                       &outStart, &outLen);
    if (fd < 0) {
        pinfo("[video]: failed to open URL: %s %d (%s)", url.c_str(), fd, strerror(errno));
        return;
    }

    _mediaData.fd = fd;

    VROMediaData *mediaData = &_mediaData;

    AMediaExtractor *extractor = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSourceFd(extractor, mediaData->fd,
                                                         static_cast<off64_t>(outStart),
                                                         static_cast<off64_t>(outLen));
    close(mediaData->fd);
    if (err != AMEDIA_OK) {
        pinfo("[video]: setDataSource error: %d", err);
        return;
    }

    int numTracks = AMediaExtractor_getTrackCount(extractor);

    pinfo("[video]: input has %d tracks", numTracks);
    for (int i = 0; i < numTracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *s = AMediaFormat_toString(format);
        pinfo("[video]: track %d format: %s", i, s);

        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            pinfo("[video]: no mime type");
            return;
        }
        else if (!strncmp(mime, "video/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);

            AMediaCodec *videoCodec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(videoCodec, format, mediaData->window, NULL, 0);

            mediaData->videoCodec = videoCodec;
            mediaData->videoCodecTrack = i;
            AMediaCodec_start(videoCodec);
        }
        else if (!strncmp(mime, "audio/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);

            AMediaCodec *audioCodec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(audioCodec, format, NULL, NULL, 0);

            mediaData->audioCodec = audioCodec;
            mediaData->audioCodecTrack = i;
            AMediaCodec_start(audioCodec);
        }

        AMediaFormat_delete(format);
    }

    mediaData->extractor = extractor;
    mediaData->renderStart = -1;
    mediaData->sawInputEOS = false;
    mediaData->sawOutputEOS = false;
    mediaData->isPlaying = false;
    mediaData->renderOnce = true;

    _looper = new VROVideoLooper();
    _looper->post(kMsgCodecBuffer, &_mediaData);
}

void VROVideoTextureAndroid::prewarm() {

}

void VROVideoTextureAndroid::onFrameWillRender(const VRORenderContext &context) {
    if (_mediaData.sawOutputEOS) {
        pause();
        if (_loop) {
            seekToTime(0);
            play();
        }
    }
}

void VROVideoTextureAndroid::onFrameDidRender(const VRORenderContext &context) {

}

void VROVideoTextureAndroid::pause() {
    _looper->post(kMsgPause, &_mediaData);
    _paused = true;
}

void VROVideoTextureAndroid::play() {
    _looper->post(kMsgResume, &_mediaData);
    _paused = false;
}

bool VROVideoTextureAndroid::isPaused() {
    return _paused;
}

void VROVideoTextureAndroid::seekToTime(int seconds) {
    VROVideoSeek *seek = (VROVideoSeek *) malloc(sizeof(VROVideoSeek));
    seek->data = &_mediaData;
    seek->seekTime = seconds * 1000000;

    _looper->post(kMsgSeek, seek);
}

void VROVideoTextureAndroid::setMuted(bool muted) {

}

void VROVideoTextureAndroid::setVolume(float volume) {

}

void VROVideoTextureAndroid::setLoop(bool loop) {
    _loop = loop;
}

VROVideoLooper::~VROVideoLooper() {
    delete (_audio);
}

void VROVideoLooper::handle(int what, void *obj) {
    if (_audio == nullptr) {
        _audio = new VROBufferAudioPlayer(48000, 960);
        //_audio->playClip();
    }
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork((VROMediaData *) obj);
            break;

        case kMsgDecodeDone: {
            VROMediaData *d = (VROMediaData *) obj;

            if (d->videoCodec != nullptr) {
                AMediaCodec_stop(d->videoCodec);
                AMediaCodec_delete(d->videoCodec);
            }
            if (d->audioCodec != nullptr) {
                AMediaCodec_stop(d->audioCodec);
                AMediaCodec_delete(d->audioCodec);
            }

            AMediaExtractor_delete(d->extractor);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;

        case kMsgSeek: {
            VROVideoSeek *seek = (VROVideoSeek *) obj;
            VROMediaData *d = (VROMediaData *) seek->data;
            AMediaExtractor_seekTo(d->extractor, seek->seekTime, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);

            if (d->videoCodec != nullptr) {
                AMediaCodec_flush(d->videoCodec);
            }
            if (d->audioCodec != nullptr) {
                AMediaCodec_flush(d->audioCodec);
            }
            d->renderStart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderOnce = true;
                post(kMsgCodecBuffer, d);
            }

            pinfo("[video]: seeked to %llu", seek->seekTime);
            free(seek);
        }
            break;

        case kMsgPause: {
            VROMediaData *d = (VROMediaData *) obj;
            if (d->isPlaying) {
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;

        case kMsgResume: {
            VROMediaData *d = (VROMediaData *) obj;
            if (!d->isPlaying) {
                d->renderStart = -1;
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;
    }
}

int64_t systemnanotime() {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}

void VROVideoLooper::writeToCodec(AMediaCodec *codec, AMediaExtractor *extractor,
                                  bool *outSawInputEOS) {
    // Retrieve an input buffer from the codec, and read the sample
    // data into it
    ssize_t bufferIndex = AMediaCodec_dequeueInputBuffer(codec, 2000);
    if (bufferIndex >= 0) {
        size_t bufsize;
        uint8_t *buf = AMediaCodec_getInputBuffer(codec, bufferIndex, &bufsize);

        auto sampleSize = AMediaExtractor_readSampleData(extractor, buf, bufsize);
        if (sampleSize < 0) {
            sampleSize = 0;
            *outSawInputEOS = true;
            pinfo("[video]: sample size 0, end of stream reached");
        }

        // Now that the sample data is in the input buffer, queue it back into the
        // codec for processing (decoding)
        auto presentationTimeUs = AMediaExtractor_getSampleTime(extractor);
        AMediaCodec_queueInputBuffer(codec, bufferIndex, 0, sampleSize, presentationTimeUs,
                                     *outSawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM
                                                     : 0);
    }
}

void VROVideoLooper::readFromVideoCodec(AMediaCodec *codec, bool *renderOnce, int64_t *renderStart,
                                        bool *sawOutputEOS) {
    AMediaCodecBufferInfo info;
    auto status = AMediaCodec_dequeueOutputBuffer(codec, &info, 0);
    if (status >= 0) {
        if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
            pinfo("[video]: output EOS");
            *sawOutputEOS = true;
        }
        int64_t presentationNano = info.presentationTimeUs * 1000;
        if (*renderStart < 0) {
            *renderStart = systemnanotime() - presentationNano;
        }
        int64_t delay = (*renderStart + presentationNano) - systemnanotime();
        if (delay > 0) {
            usleep(delay / 1000);
        }
        AMediaCodec_releaseOutputBuffer(codec, status, info.size != 0);
        if (*renderOnce) {
            *renderOnce = false;
            return;
        }
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        pinfo("[video]: output buffers changed");
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = AMediaCodec_getOutputFormat(codec);
        pinfo("[video]: format changed to: %s", AMediaFormat_toString(format));
        AMediaFormat_delete(format);
    }
    else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
    }
    else {
        pinfo("[video]: unexpected info code: %zd", status);
    }
}

void VROVideoLooper::readFromAudioCodec(AMediaCodec *codec, bool *sawOutputEOS) {
    AMediaCodecBufferInfo info;
    ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(codec, &info, 0);
    if (bufferIndex >= 0) {
        if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
            pinfo("[video]: output EOS");
            *sawOutputEOS = true;
        }

        size_t bufSize;
        uint8_t *audioBuffer = AMediaCodec_getOutputBuffer(codec, bufferIndex, &bufSize);
        _audio->queueAudio((char *) audioBuffer, info.size);

        /*
        int64_t presentationNano = info.presentationTimeUs * 1000;
        if (*renderStart < 0) {
            *renderStart = systemnanotime() - presentationNano;
        }
        int64_t delay = (*renderStart + presentationNano) - systemnanotime();
        if (delay > 0) {
            usleep(delay / 1000);
        }
         */

        AMediaCodec_releaseOutputBuffer(codec, bufferIndex, false);
    }
    else if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        pinfo("[audio]: output buffers changed");
    }
    else if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = AMediaCodec_getOutputFormat(codec);
        pinfo("[audio]: format changed to: %s", AMediaFormat_toString(format));
        AMediaFormat_delete(format);
    }
    else if (bufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
    }
    else {
        pinfo("[audio]: unexpected info code: %zd", bufferIndex);
    }
}

void VROVideoLooper::doCodecWork(VROMediaData *d) {

    // In this block we read encoded track data from the extractor, and
    // write into the codec for decoding
    if (!d->sawInputEOS) {

        // Get the track index of the current sample
        int trackIndex = AMediaExtractor_getSampleTrackIndex(d->extractor);
        if (trackIndex == -1) {
            // If there are no more samples, then queue one more buffer
            // in each codec so we can pass the END_OF_STREAM flag
            pinfo("[video]: track index -1, end of stream reached");

            if (d->videoCodec) {
                writeToCodec(d->videoCodec, d->extractor, &d->sawInputEOS);
            }
            if (d->audioCodec) {
                writeToCodec(d->audioCodec, d->extractor, &d->sawInputEOS);
            }
        }
        else {
            AMediaCodec *codec = d->codecForTrack(trackIndex);
            writeToCodec(codec, d->extractor, &d->sawInputEOS);
        }

        // Advance to the next sample
        AMediaExtractor_advance(d->extractor);
    }

    // In this block we read decoded track data from the codecs, and write it
    // into the audio/video sinks
    if (!d->sawOutputEOS) {

        // Read video data. To read video we just have to dequeue the output buffer
        // and pass 'true' to the release function; since we attached a surface when configuring the
        // codec, the data will be automatically passed to the surface for rendering
        if (d->videoCodec) {
            bool wasRenderOnce = d->renderOnce;
            readFromVideoCodec(d->videoCodec, &d->renderOnce, &d->renderStart, &d->sawOutputEOS);
            if (wasRenderOnce && !d->renderOnce) {
                return;
            }
        }
        if (d->audioCodec) {
            readFromAudioCodec(d->audioCodec, &d->sawOutputEOS);
        }
    }

    if (!d->sawInputEOS || !d->sawOutputEOS) {
        post(kMsgCodecBuffer, d);
    }
}

VROBufferAudioPlayer::VROBufferAudioPlayer(int sampleRate, int bufferSize) :
    _sampleRate(sampleRate * 1000),
    _bufferSize(bufferSize) {

    SLresult result;

    // Create the Open SLES audio and engine interfaces
    result = slCreateEngine(&_audio, 0, NULL, 0, NULL, NULL);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_audio)->Realize(_audio, SL_BOOLEAN_FALSE);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_audio)->GetInterface(_audio, SL_IID_ENGINE, &_audioEngine);
    passert(SL_RESULT_SUCCESS == result);

    // Create and realize the output mix
    result = (*_audioEngine)->CreateOutputMix(_audioEngine, &_outputMix, 0, NULL, NULL);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_outputMix)->Realize(_outputMix, SL_BOOLEAN_FALSE);
    passert(SL_RESULT_SUCCESS == result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPCM = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    formatPCM.samplesPerSec = _sampleRate;
    SLDataSource audioSource = {&loc_bufq, &formatPCM};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, _outputMix};
    SLDataSink audioSink = {&loc_outmix, NULL};

    /*
     Create the audio player.
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*_audioEngine)->CreateAudioPlayer(_audioEngine, &_player, &audioSource, &audioSink, 2,
                                                ids, req);
    passert(SL_RESULT_SUCCESS == result);

    // Retrieve, the player, play state, buffer queue, and volume interfaces
    result = (*_player)->Realize(_player, SL_BOOLEAN_FALSE);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_player)->GetInterface(_player, SL_IID_PLAY, &_playState);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_player)->GetInterface(_player, SL_IID_BUFFERQUEUE, &_bufferQueue);
    passert(SL_RESULT_SUCCESS == result);

    result = (*_player)->GetInterface(_player, SL_IID_VOLUME, &_volume);
    passert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    //result = (*_bufferQueue)->RegisterCallback(_bufferQueue, playerCallback, NULL);
    //passert(SL_RESULT_SUCCESS == result);
    //(void)result;

    // Set the player's state to playing
    result = (*_playState)->SetPlayState(_playState, SL_PLAYSTATE_PLAYING);
    passert(SL_RESULT_SUCCESS == result);
}

VROBufferAudioPlayer::~VROBufferAudioPlayer() {

}

short *VROBufferAudioPlayer::createResampledBuffer(const char *source, int sourceSize,
                                                   uint32_t sourceRate,
                                                   unsigned *outSize) {
    if (_sampleRate == 0) {
        return NULL;
    }

    // Simple up-sampling, must be divisible
    if (_sampleRate % sourceRate) {
        return NULL;
    }

    int upSampleRate = _sampleRate / sourceRate;

    int32_t sourceSampleCount = sourceSize >> 1;
    short *src = (short *) source;

    // TODO LEAK
    short *resampleBuf = (short *) malloc((sourceSampleCount * upSampleRate) << 1);
    if (resampleBuf == NULL) {
        return resampleBuf;
    }
    short *workBuf = resampleBuf;
    for (int sample = 0; sample < sourceSampleCount; sample++) {
        for (int dup = 0; dup < upSampleRate; dup++) {
            *workBuf++ = src[sample];
        }
    }

    *outSize = (sourceSampleCount * upSampleRate) << 1;     // sample format is 16 bit
    return resampleBuf;
}

void VROBufferAudioPlayer::playClip() {
    unsigned nextSize;
    short *nextBuffer = createResampledBuffer(hello, sizeof(hello), SL_SAMPLINGRATE_8, &nextSize);
    if (!nextBuffer) {
        nextBuffer = (short *) hello;
        nextSize = sizeof(hello);
    }

    if (nextSize > 0) {
        // here we only enqueue one buffer because it is a long clip,
        // but for streaming playback we would typically enqueue at least 2 buffers to start
        SLresult result;
        result = (*_bufferQueue)->Enqueue(_bufferQueue, nextBuffer, nextSize);
    }
}

void VROBufferAudioPlayer::queueAudio(const char *audio, int size) {
    //unsigned resampledSize;
    //short *resampledAudio = createResampledBuffer(audio, size, SL_SAMPLINGRATE_8, &resampledSize);

    SLresult result;
    result = (*_bufferQueue)->Enqueue(_bufferQueue, audio, size);
}