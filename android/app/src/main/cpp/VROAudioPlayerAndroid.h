//
//  VROAudioPlayerAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROAUDIOPLAYERANDROID_H
#define ANDROID_VROAUDIOPLAYERANDROID_H

#include "VROAudioPlayer.h"
#include <memory>
#include <string>

// TODO delete
#include "vr/gvr/capi/include/gvr_audio.h"

class VROAVPlayer;

class VROAudioPlayerAndroid : public VROAudioPlayer {

public:

    VROAudioPlayerAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROAudioPlayerAndroid();

    void setLoop(bool loop);
    void play();
    void pause();
    void setVolume(float volume);
    void setMuted(bool muted);
    void seekToTime(float seconds);

private:

    VROAVPlayer *_player;

};

#endif //ANDROID_VROAUDIOPLAYERANDROID_H
