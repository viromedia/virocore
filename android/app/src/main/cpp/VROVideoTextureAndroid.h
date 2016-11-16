//
//  VROVideoTextureAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROVIDEOTEXTUREANDROID_H
#define ANDROID_VROVIDEOTEXTUREANDROID_H

#include "VROVideoTexture.h"
#include "VROOpenGL.h"

#include <vector>
#include <queue>
#include <android/native_window_jni.h>
#include "VROLooper.h"
#include "media/NdkMediaCodec.h"
#include "media/NdkMediaExtractor.h"

class VROPCMAudioPlayer;

enum class VROCodecType {
    Video,
    Audio
};

class VROCodecOutputBuffer {
public:
    ssize_t index;
    AMediaCodecBufferInfo info;

    VROCodecOutputBuffer(ssize_t index, AMediaCodecBufferInfo info) {
        this->index = index;
        this->info = info;
    }
};

class VROCodec {
public:

    VROCodec(VROCodecType type, AMediaCodec *codec, int track) :
            _type(type), _codec(codec), _track(track) {}

    VROCodecType getType() const { return _type; }
    AMediaCodec *getCodec() const { return _codec; }
    int getTrack() const { return _track; }

    /*
     Lifecycle operations.
     */
    void stop() {
        AMediaCodec_stop(_codec);
        AMediaCodec_delete(_codec);
    }
    void flush() {
        AMediaCodec_flush(_codec);
    }

    /*
     Input (to codec) buffer operations.
     */
    ssize_t dequeueInputBuffer(int64_t timeoutUs) {
        return AMediaCodec_dequeueInputBuffer(_codec, timeoutUs);
    }
    uint8_t *getInputBuffer(ssize_t bufferIndex, size_t *outBufSize) {
        return AMediaCodec_getInputBuffer(_codec, bufferIndex, outBufSize);
    }
    media_status_t queueInputBuffer(size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags) {
        return AMediaCodec_queueInputBuffer(_codec, idx, offset, size, time, flags);
    }

    /*
     Output (from codec) buffer operations.
     */
    ssize_t dequeueOutputBuffer(AMediaCodecBufferInfo *info, int64_t timeoutUs) {
        return AMediaCodec_dequeueOutputBuffer(_codec, info, timeoutUs);
    }
    media_status_t releaseOutputBuffer(size_t idx, bool render) {
        return AMediaCodec_releaseOutputBuffer(_codec, idx, render);
    }
    AMediaFormat* getOutputFormat() {
        return AMediaCodec_getOutputFormat(_codec);
    }
    uint8_t* getOutputBuffer(size_t idx, size_t *out_size) {
        return AMediaCodec_getOutputBuffer(_codec, idx, out_size);
    }

    /*
     Ready (to render) output buffer operations.
     */
    bool hasOutputBuffer() {
        return !readyOutputBuffers.empty();
    }
    void pushOutputBuffer(VROCodecOutputBuffer buffer) {
        readyOutputBuffers.push(buffer);
    }
    VROCodecOutputBuffer nextOutputBuffer() {
        return readyOutputBuffers.front();
    }
    void popOutputBuffer() {
        readyOutputBuffers.pop();
    }

private:

    const VROCodecType _type;
    AMediaCodec *_codec;
    const int _track;

    std::queue<VROCodecOutputBuffer> readyOutputBuffers;

};

class VROMediaData {

public:

    int fd;
    ANativeWindow *window;
    AMediaExtractor *extractor;
    VROCodec *videoCodec = nullptr;
    VROCodec *audioCodec = nullptr;

    int64_t renderStart;
    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderOnce;

    virtual ~VROMediaData() {
        delete (videoCodec);
        delete (audioCodec);
    }

    VROCodec *codecForTrack(int track) {
        if (videoCodec && track == videoCodec->getTrack()) {
            return videoCodec;
        }
        else if (audioCodec && track == audioCodec->getTrack()) {
            return audioCodec;
        }
        else {
            return nullptr;
        }
    }
};

typedef struct {
    VROMediaData *data;
    int64_t seekTime;
} VROVideoSeek;

class VROVideoLooper;

class VROVideoTextureAndroid : public VROVideoTexture {

public:

    VROVideoTextureAndroid();
    virtual ~VROVideoTextureAndroid();

    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           VRODriver &driver);

    void prewarm();

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void pause();
    void play();
    bool isPaused();
    void seekToTime(int seconds);

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

private:

    VROMediaData _mediaData;
    VROVideoLooper *_looper;
    bool _paused;
    bool _loop;

    void createVideoTexture();

};

/*
 * Runs on another thread decoding the video. Post messages
 * to this thread using the VROLooper interface.
 */
class VROVideoLooper : public VROLooper {

public:

    //VROVideoLooper();
    virtual ~VROVideoLooper();

    void handle(int what, void* obj);

private:

    VROPCMAudioPlayer *_audio = nullptr;

    void doCodecWork(VROMediaData *d);

    void writeToCodec(VROCodec *codec, AMediaExtractor *extractor, bool *outSawInputEOS);
    void readFromCodec(VROCodec *codec, bool *outSawOutputEOS);

    void writeToSinks(VROCodec *videoCodec, VROCodec *audioCodec, int64_t *renderStart);
    void writeToVideoSink(VROCodec *codec, int64_t *renderStart);
    void writeToAudioSink(VROCodec *codec, int64_t *renderStart);

    int64_t computeDelayToRender(VROCodecOutputBuffer buffer, int64_t *renderStart);
    void renderVideo(VROCodecOutputBuffer buffer, VROCodec *codec);
    void renderAudio(VROCodecOutputBuffer buffer, VROCodec *codec);

};


#endif //ANDROID_VROVIDEOTEXTUREANDROID_H
