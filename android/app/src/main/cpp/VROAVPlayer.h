//
//  VROAVPlayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROAVPLAYER_H
#define ANDROID_VROAVPLAYER_H

#include <jni.h>
#include <stdint.h>
#include <VROVideoDelegateInternal.h>
#include <memory>
#include "VROOpenGL.h"

class VROAVPlayer {

public:

    VROAVPlayer();
    virtual ~VROAVPlayer();

    bool setDataSourceURL(const char *fileOrURL);
    bool setDataSourceAsset(const char *asset);
    void setSurface(GLuint textureId);

    void pause();
    void play();
    bool isPaused();
    void seekToTime(float seconds);

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

    void reset();
    void setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate) {
        _delegate = delegate;
    }
    std::weak_ptr<VROVideoDelegateInternal> getDelegate(){
        return _delegate;
    }

private:
    jobject _javPlayer;
    jobject _jsurface;

    void bindVideoSink();

    /**
     * VideoDelegateInternal provided by VROVideoTexture for triggering video callbacks.
     */
    std::weak_ptr<VROVideoDelegateInternal> _delegate;
};

#endif //ANDROID_VROAVPLAYER_H
