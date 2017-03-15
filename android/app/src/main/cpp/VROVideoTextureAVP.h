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

/*
 Renders video to a texture and plays the associated audio.
 The VROVideoLooper is the underlying workhorse class for
 rendering video.
 */
class VROVideoTextureAVP : public VROVideoTexture, public VROAVPlayerDelegate {

public:

    VROVideoTextureAVP();
    virtual ~VROVideoTextureAVP();

    /*
     Must be invoked from the rendering thread after construction.
     */
    void bindSurface();

    /*
     Standard load video function: loads from URL. Frame synchronizer is not
     required (inherited argument from superclass).
     */
    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           VRODriver &driver);

    /*
     Load from a file at the given URL or an asset with the given name.
     */
    void loadVideoFromURL(std::string url, VRODriver &driver);
    void loadVideoFromAsset(std::string asset, VRODriver &driver);

    void prewarm();

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void pause();
    void play();
    bool isPaused();

    void seekToTime(int seconds);
    int getCurrentTimeInSeconds();
    int getVideoDurationInSeconds();

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

    void setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate);

    #pragma mark - VROAVPlayerDelegate
    virtual void onPrepared();
    virtual void onFinished();

private:

    VROAVPlayer *_player;
    GLuint _textureId;

};

#endif //ANDROID_VROVIDEOTEXTUREAVP_H
