//
//  VROAVPlayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

    virtual void willBuffer() = 0;

    virtual void didBuffer() = 0;

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
    void setSurface(GLuint textureId);

    void pause();
    void play();
    bool isPaused();

    void seekToTime(float seconds);
    float getCurrentTimeInSeconds();
    float getVideoDurationInSeconds();

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
