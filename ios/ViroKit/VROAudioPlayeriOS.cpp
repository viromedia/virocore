//
//  VROAudioPlayeriOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayeriOS.h"
#include "VROData.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VROMath.h"

VROAudioPlayeriOS::VROAudioPlayeriOS(std::string url, bool isLocalUrl) :
    _playVolume(1.0) {
    if (isLocalUrl) {
        _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]
                                                     error:NULL];
        [_player prepareToPlay];
        _delegate->soundIsReady();
    } else {
        // download to file
        NSURL *urlObj = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
        downloadDataWithURL(urlObj, ^(NSData *data, NSError *error) {
            _player = [[AVAudioPlayer alloc] initWithData:data error:NULL];
            _delegate->soundIsReady();
        });
    }
}

VROAudioPlayeriOS::VROAudioPlayeriOS(std::shared_ptr<VROData> data) :
    _playVolume(1.0) {
    
    _player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:data->getData() length:data->getDataLength()]
                                            error:NULL];
    [_player prepareToPlay];
}

std::shared_ptr<VROAudioPlayeriOS> VROAudioPlayeriOS::create(std::shared_ptr<VROSoundData> data) {
    std::shared_ptr<VROAudioPlayeriOS> player = std::make_shared<VROAudioPlayeriOS>(data);
    player->setup();
    return player;
}

VROAudioPlayeriOS::VROAudioPlayeriOS(std::shared_ptr<VROSoundData> data) :
_playVolume(1.0) {
    _data = data;
}

VROAudioPlayeriOS::~VROAudioPlayeriOS() {
    if (_player) {
        [_player stop];
    }
}

void VROAudioPlayeriOS::setLoop(bool loop) {
    _player.numberOfLoops = loop ? -1 : 0;
}

void VROAudioPlayeriOS::setMuted(bool muted) {
    _muted = muted;
    if (_player) {
        _player.volume = muted ? 0 : _playVolume;
    }
}

void VROAudioPlayeriOS::seekToTime(float seconds) {
    if (_player) {
        seconds = clamp(seconds, 0, _player.duration);
        _player.currentTime = seconds;
    }
}

void VROAudioPlayeriOS::setVolume(float volume) {
    _playVolume = volume;
    if (_player) {
        _player.volume = volume;
    }
}

void VROAudioPlayeriOS::play() {
    _paused = false;
    if (_player) {
        _player.volume = _playVolume;
        [_player play];
    }
}

void VROAudioPlayeriOS::pause() {
    _paused = true;
    if (_player) {
        doFadeThenPause();
    }
}

void VROAudioPlayeriOS::setup() {
    if (_data) {
        _data->setDelegate(shared_from_this());
    }
}

void VROAudioPlayeriOS::doFadeThenPause() {
    if (_player.volume > 0.1) {
        _player.volume = _player.volume - 0.1;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            doFadeThenPause();
        });
    }
    else {
        [_player pause];
    }
}

void VROAudioPlayeriOS::dataIsReady() {
    if (!_player) {
        _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:_data->getLocalFilePath().c_str()]]
                                                         error:NULL];
        [_player prepareToPlay];
        if (!_paused) {
            _player.volume = _muted ? 0 : _playVolume;
            [_player play];
        }
    }
}
