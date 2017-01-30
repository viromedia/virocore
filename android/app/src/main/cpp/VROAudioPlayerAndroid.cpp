//
//  VROAudioPlayerAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayerAndroid.h"

VROAudioPlayerAndroid::VROAudioPlayerAndroid(std::string fileName) {
    _player = new VROAVPlayer();
    // this URL can either be an external web link or a local file link
    _player->setDataSourceURL(fileName.c_str());
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

void VROAudioPlayerAndroid::setDelegate(std::shared_ptr<VROSoundDelegateInternal> delegate) {
    _delegate = delegate;
    std::shared_ptr<VROAVPlayerDelegate> avDelegate =
            std::dynamic_pointer_cast<VROAVPlayerDelegate>(shared_from_this());
    _player->setDelegate(avDelegate);
}

#pragma mark - VROAVPlayerDelegate
void VROAudioPlayerAndroid::onPrepared() {
    if (_delegate) {
        _delegate->soundIsReady();
    }
}

void VROAudioPlayerAndroid::onFinished() {
    if (_delegate) {
        _delegate->soundDidFinish();
    }
}