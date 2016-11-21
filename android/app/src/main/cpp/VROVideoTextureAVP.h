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

/*
 Renders video to a texture and plays the associated audio.
 The VROVideoLooper is the underlying workhorse class for
 rendering video.
 */
class VROVideoTextureAVP : public VROVideoTexture {

public:

    VROVideoTextureAVP();
    virtual ~VROVideoTextureAVP();

    /*
     Standard load video function: loads from URL.
     */
    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           VRODriver &driver);

    /*
     Load from an asset with the given name.
     */
    void loadVideoFromAsset(std::string asset, VRODriver &driver);

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

    VROAVPlayer *_player;
    GLuint _textureId;

    void bindSurface();

};

#endif //ANDROID_VROVIDEOTEXTUREAVP_H
