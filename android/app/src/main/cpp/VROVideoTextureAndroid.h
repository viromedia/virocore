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

#include <android/native_window_jni.h>
#include "VROLooper.h"
#include "media/NdkMediaCodec.h"
#include "media/NdkMediaExtractor.h"

class VROMediaData {

public:

    int fd;
    ANativeWindow *window;
    AMediaExtractor *extractor;
    AMediaCodec *videoCodec;
    AMediaCodec *audioCodec;
    int videoCodecTrack;
    int audioCodecTrack;
    int64_t renderStart;
    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderOnce;

    AMediaCodec *codecForTrack(int track) {
        if (track == videoCodecTrack) {
            return videoCodec;
        }
        else if (track == audioCodecTrack) {
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

    virtual void handle(int what, void* obj);
    void doCodecWork(VROMediaData *d);

    void writeToCodec(AMediaCodec *codec, AMediaExtractor *extractor, bool *outSawInputEOS);
    void readFromVideoCodec(AMediaCodec *codec, bool *renderOnce, int64_t *renderStart, bool *outSawOutputEOS);
    void readFromAudioCodec(AMediaCodec *codec, bool *outSawOutputEOS);

};

#endif //ANDROID_VROVIDEOTEXTUREANDROID_H
