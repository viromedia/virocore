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

VROAudioPlayeriOS::~VROAudioPlayeriOS() {
    [_player stop];
}

void VROAudioPlayeriOS::setLoop(bool loop) {
    _player.numberOfLoops = loop ? -1 : 0;
}

void VROAudioPlayeriOS::setMuted(bool muted) {
    if (muted) {
        _player.volume = 0;
    }
    else {
        _player.volume = _playVolume;
    }
}

void VROAudioPlayeriOS::seekToTime(float seconds) {
    _player.currentTime = seconds;
}

void VROAudioPlayeriOS::setVolume(float volume) {
    _playVolume = volume;
    _player.volume = volume;
}

void VROAudioPlayeriOS::play() {
    _player.volume = _playVolume;
    [_player play];
}

void VROAudioPlayeriOS::pause() {
    doFadeThenPause();
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

