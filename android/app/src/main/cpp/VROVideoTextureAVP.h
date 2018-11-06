//
//  VROVideoTextureAVP.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROVIDEOTEXTUREAVP_H
#define ANDROID_VROVIDEOTEXTUREAVP_H

#include "VROVideoTexture.h"
#include "VROOpenGL.h"
#include "VROAVPlayer.h"
#include <android/native_window_jni.h>
#include "VROFrameSynchronizer.h"

class VRODriverOpenGL;

/*
 Renders video to a texture and plays the associated audio.
 The VROVideoLooper is the underlying workhorse class for
 rendering video.
 */
class VROVideoTextureAVP : public VROVideoTexture, public VROAVPlayerDelegate {

public:

    /*
     Construct the video texture. After construction, init() must be
     called from the UI thread, and bindSurface() from the rendering thread.
     */
    VROVideoTextureAVP(VROStereoMode state = VROStereoMode::None);
    virtual ~VROVideoTextureAVP();

    /*
     Must be invoked from the UI thread after construction.
     */
    void init();

    /*
     Must be invoked from the rendering thread after construction.
     */
    void bindSurface(std::shared_ptr<VRODriverOpenGL> driver);

    /*
     Standard load video function: loads from URL. Frame synchronizer is not
     required (inherited argument from superclass).
     */
    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           std::shared_ptr<VRODriver> driver);

    /*
     Load from a file at the given URL or an asset with the given name.
     */
    void loadVideoFromURL(std::string url, std::shared_ptr<VRODriver> driver);
    void prewarm();

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void pause();
    void play();
    bool isPaused();

    void seekToTime(float seconds);
    float getCurrentTimeInSeconds();
    float getVideoDurationInSeconds();

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

    void setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate);

    #pragma mark - VROAVPlayerDelegate

    virtual void willBuffer();
    virtual void didBuffer();
    virtual void onPrepared();
    virtual void onFinished();
    virtual void onError(std::string error);

private:

    VROAVPlayer *_player;
    GLuint _textureId;
    std::weak_ptr<VRODriverOpenGL> _driver;

};

#endif //ANDROID_VROVIDEOTEXTUREAVP_H
