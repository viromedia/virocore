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
    void seekToTime(int seconds);

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

private:

    jobject _javPlayer;

};

#endif //ANDROID_VROAVPLAYER_H
