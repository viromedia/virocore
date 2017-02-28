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

std::shared_ptr<VROAudioPlayerAndroid> VROAudioPlayerAndroid::create(
        std::shared_ptr<VROSoundData> data) {
    std::shared_ptr<VROAudioPlayerAndroid> player = std::make_shared<VROAudioPlayerAndroid>(data);
    player->setup();
    return player;
}

VROAudioPlayerAndroid::VROAudioPlayerAndroid(std::shared_ptr<VROSoundData> data) {
    _player = new VROAVPlayer();
    _data = data;
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
    // this is just a generic duration from the Android MediaPlayer
    int totalDuration = _player->getVideoDurationInSeconds();
    if (seconds > totalDuration) {
        seconds = totalDuration;
    } else if (seconds < 0) {
        seconds = 0;
    }
    _player->seekToTime(seconds);
}

void VROAudioPlayerAndroid::setDelegate(std::shared_ptr<VROSoundDelegateInternal> delegate) {
    _delegate = delegate;
    std::shared_ptr<VROAVPlayerDelegate> avDelegate =
            std::dynamic_pointer_cast<VROAVPlayerDelegate>(shared_from_this());
    _player->setDelegate(avDelegate);
}

void VROAudioPlayerAndroid::setup() {
    if (_data) {
        _data->setDelegate(shared_from_this());
    }
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

#pragma mark - VROSoundDataDelegate
void VROAudioPlayerAndroid::dataIsReady() {
    if (_player) {
        _player->setDataSourceURL(_data->getLocalFilePath().c_str());
    }
}