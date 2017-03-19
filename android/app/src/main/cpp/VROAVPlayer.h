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

class VROAVPlayerDelegate {

public:
    VROAVPlayerDelegate() {}
    virtual ~VROAVPlayerDelegate() {}

    // this is called when the AVPlayer has finished preparing the media
    virtual void onPrepared() = 0;

    // this is called when the AVPlayer has finished playing the media
    virtual void onFinished() = 0;

    // this is called when the AVPlayer has encountered an error
    virtual void onError(std::string error) = 0;
};

class VROAVPlayer {

public:

    VROAVPlayer();
    virtual ~VROAVPlayer();

    bool setDataSourceURL(const char *resourceOrUrl);
    bool setDataSourceAsset(const char *asset);
    void setSurface(GLuint textureId);

    void pause();
    void play();
    bool isPaused();

    void seekToTime(int seconds);
    int getCurrentTimeInSeconds();
    int getVideoDurationInSeconds();

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

    void reset();
    void setDelegate(std::shared_ptr<VROAVPlayerDelegate> delegate) {
        _delegate = delegate;
    }
    std::weak_ptr<VROAVPlayerDelegate> getDelegate(){
        return _delegate;
    }

private:

    jobject _javPlayer;
    jobject _jsurface;
    GLuint _textureId;

    void bindVideoSink();

    std::weak_ptr<VROAVPlayerDelegate> _delegate;
};

#endif //ANDROID_VROAVPLAYER_H
