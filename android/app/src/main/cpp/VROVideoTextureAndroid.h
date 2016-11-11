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

typedef struct {
    int fd;
    ANativeWindow* window;
    AMediaExtractor* ex;
    AMediaCodec *codec;
    int64_t renderstart;
    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderonce;
} VROVideoData;

typedef struct {
    VROVideoData *data;
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

    VROVideoData _data;
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
    void doCodecWork(VROVideoData *d);

};

#endif //ANDROID_VROVIDEOTEXTUREANDROID_H
