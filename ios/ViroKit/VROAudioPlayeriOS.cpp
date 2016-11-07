//
//  VROAudioPlayeriOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayeriOS.h"
#include "VROData.h"

VROAudioPlayeriOS::VROAudioPlayeriOS() :
    _player(nullptr) {
    
}

VROAudioPlayeriOS::~VROAudioPlayeriOS() {
    
}

void VROAudioPlayeriOS::setTrack(std::string url, int loopCount) {
    if (_player) {
        [_player stop];
    }
    
    _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]
                                                     error:NULL];
    _player.numberOfLoops = loopCount;
    [_player prepareToPlay];
}

void VROAudioPlayeriOS::setTrack(std::shared_ptr<VROData> data, int loopCount) {
    if (_player) {
        [_player stop];
    }
    
    _player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:data->getData() length:data->getDataLength()]
                                            error:NULL];
    _player.numberOfLoops = loopCount;
    [_player prepareToPlay];
}

void VROAudioPlayeriOS::stop() {
    doFadeThenStop();
}

void VROAudioPlayeriOS::play() {
    _player.volume = 1.0;
    [_player play];
    _paused = false;
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
        _paused = true;
    }
}

void VROAudioPlayeriOS::doFadeThenStop() {
    if (_player.volume > 0.1) {
        _player.volume = _player.volume - 0.1;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            doFadeThenStop();
        });
    }
    else {
        [_player stop];
    }
}
