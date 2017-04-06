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

/*
 Wraps an output buffer from a codec. Output buffers contain
 decoded data, ready to be rendered, and an associated
 presentation time that tells us when we should render the data.
 */
class VROCodecOutputBuffer {
public:
    ssize_t index;
    AMediaCodecBufferInfo info;

    VROCodecOutputBuffer(ssize_t index, AMediaCodecBufferInfo info) {
        this->index = index;
        this->info = info;
    }
};

/*
 Wrapper over codecs. Codecs are responsible for decoding media
 data into their raw form where they can be displayed as textures
 or played as audio. Codecs contain input buffers, where encoded
 media is written, and output buffers, where decoded media is
 read. The typical cycle for a single blob of data is:

 1. Read the encoded data from media
 2. Dequeue an input buffer: dequeueInputBuffer
 3. Write the encoded data to said buffer: getInputBuffer

 4. Wait... (while the codec decodes)
    when finished, the decoded data is placed in an output buffer

 5. Dequeue the output buffer: dequeueOutputBuffer
 6. Read the data from the output buffer: getOutputBuffer
 7. Add the output buffer to the ready to display queue: pushOutputBuffer

 8. Wait... (until the buffer's presentation time)
 9. Get the ready to display buffer: nextOutputBuffer
 10. Render the buffer using an audio or video sink
 11. Pop the buffer off the ready to display queue: popOutputBuffer
 12. Release the buffer back to the codec: releaseOutputBuffer
 */
class VROCodec {
public:

    VROCodec(VROCodecType type, const char *mime,
             AMediaFormat *format, ANativeWindow *window,
             int track) :
            _type(type), _track(track) {

        _codec = AMediaCodec_createDecoderByType(mime);
        AMediaCodec_configure(_codec, format, window, NULL, 0);
        AMediaCodec_start(_codec);
    }

    virtual ~VROCodec() {
        clear();
        AMediaCodec_stop(_codec);
        AMediaCodec_delete(_codec);
    }

    VROCodecType getType() const { return _type; }
    AMediaCodec *getCodec() const { return _codec; }
    int getTrack() const { return _track; }

    /*
     Lifecycle operations.
     */
    void flush() {
        clear();
        AMediaCodec_flush(_codec);
    }
    void clear() {
        while (!readyOutputBuffers.empty()) {
            VROCodecOutputBuffer buffer = readyOutputBuffers.front();
            releaseOutputBuffer(buffer.index, false);
            readyOutputBuffers.pop();
        }
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

/*
 Data structure used for communication between the VROVideoTextureAndroid
 and the VROVideoLooper. After the looper has started, this should only be
 modified on the looper thread.
 */
class VROMediaData {

public:

    int32_t deviceAudioSampleRate;
    int32_t deviceAudioBufferSize;
    int32_t sourceAudioSampleRate;
    int32_t audioNumChannels;

    bool muted;
    float volume;

    GLuint textureId;
    ANativeWindow *window;
    AMediaExtractor *extractor;
    VROCodec *videoCodec = nullptr;
    VROCodec *audioCodec = nullptr;

    int64_t renderStart;
    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderOnce;
    bool loop;

    virtual ~VROMediaData() {

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

/*
 More data structures used for communication with VROVideoLooper.
 */
typedef struct {
    int64_t seekTime;
} VROVideoSeek;

typedef struct {
    float volume;
} VROVideoVolume;

typedef struct {
    bool muted;
} VROVideoMute;

typedef struct {
    bool loop;
} VROVideoLoop;

class VROVideoLooper;

/*
 Renders video to a texture and plays the associated audio.
 The VROVideoLooper is the underlying workhorse class for
 rendering video.
 */
class VROVideoTextureAndroid : public VROVideoTexture {

public:

    VROVideoTextureAndroid();
    virtual ~VROVideoTextureAndroid();

    /*
     Standard load video function: loads from URL.
     */
    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           std::shared_ptr<VRODriver> driver);

    void prewarm();

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void pause();
    void play();
    bool isPaused();
    void seekToTime(float seconds);

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

private:

    VROVideoLooper *_looper;
    bool _paused;

    void killVideo();
    void loadVideo( AMediaExtractor *extractor, std::shared_ptr<VRODriver> driver);

    ANativeWindow *createVideoTexture(GLuint *textureId, std::shared_ptr<VRODriverOpenGL> driver);

};

/*
 Runs on a separate thread decoding and rendering the video.
 The VROVideoTexture communicates with this object via the
 VROLooper interface.
 */
class VROVideoLooper : public VROLooper {

public:

    virtual ~VROVideoLooper();
    virtual void quit();

    VROMediaData &getData() {
        return _mediaData;
    }
    void handle(int what, void* obj);

private:

    /*
     Top level function that extracts data from the media tracks,
     decodes them via their associated codecs, and writes the data
     into the corresponding video or audio sinks.
     */
    void doCodecWork();

    /*
     Write the latest data from the given media extractor into the
     provided codec, for decoding.
     */
    void writeToCodec(VROCodec *codec, AMediaExtractor *extractor, bool *outSawInputEOS);

    /*
     Read the latest decoded data from the given codec, placing the
     results in one of the codec's output buffers. From there the data
     can be written to an audio or video sink.
     */
    void readFromCodec(VROCodec *codec, bool *outSawOutputEOS);

    /*
     Render the first available output buffer in either of the given
     codecs, delaying until the first available buffer's presentationTime
     if necessary.
     */
    void writeToSinks(VROCodec *videoCodec, VROCodec *audioCodec, int64_t *renderStart);
    void writeToVideoSink(VROCodec *codec, int64_t *renderStart);
    void writeToAudioSink(VROCodec *codec, int64_t *renderStart);

    /*
     Compute the amount of time we need to delay before rendering
     the given buffer. If renderStart (which tells us when the video
     started playing) is not initialized, it may be initialized here.
     */
    int64_t computeDelayToRender(VROCodecOutputBuffer buffer, int64_t *renderStart);

    /*
     Render the given buffer to audio or video, immediately.
     */
    void renderVideo(VROCodecOutputBuffer buffer, VROCodec *codec);
    void renderAudio(VROCodecOutputBuffer buffer, VROCodec *codec);

    /*
     The audio player for playing audio tracks via raw PCM data.
     */
    VROPCMAudioPlayer *_audio = nullptr;

    /*
     The data associated with the video.
     */
    VROMediaData _mediaData;

};


#endif //ANDROID_VROVIDEOTEXTUREANDROID_H
