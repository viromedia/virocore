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
#include "vr/gvr/capi/include/gvr_audio.h"

class VROAudioPlayerAndroid : public VROAudioPlayer {

public:

    VROAudioPlayerAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROAudioPlayerAndroid();

    void setLoop(bool loop);
    void play();
    void pause();
    void setVolume(float volume);

private:

    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::string _fileName;
    bool _loop;
    float _volume;

    /*
     GVR ID for the currently playing sound. This gets rewritten each time play()
     is invoked.
     */
    gvr::AudioSourceId _audioId;

};

#endif //ANDROID_VROAUDIOPLAYERANDROID_H
