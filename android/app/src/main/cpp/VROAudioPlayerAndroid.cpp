//
//  VROAudioPlayerAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayerAndroid.h"
#include "VROLog.h"
#include "VROAVPlayer.h"

VROAudioPlayerAndroid::VROAudioPlayerAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio) {
    _player = new VROAVPlayer();
    _player->setDataSourceAsset(fileName.c_str());
}

VROAudioPlayerAndroid::~VROAudioPlayerAndroid() {
    delete (_player);
}

void VROAudioPlayerAndroid::setLoop(bool loop) {
    _player->setLoop(loop);
}

void VROAudioPlayerAndroid::play() {
    _player->play();
}

void VROAudioPlayerAndroid::pause() {
    _player->pause();
}

void VROAudioPlayerAndroid::setVolume(float volume) {
    _player->setVolume(volume);
}

void VROAudioPlayerAndroid::setMuted(bool muted) {
    _player->setMuted(muted);
}

void VROAudioPlayerAndroid::seekToTime(float seconds) {
    _player->seekToTime(seconds);
}