//
//  VROAudioPlayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayer.h"

VROAudioPlayer::VROAudioPlayer() :
    _player(nullptr), _paused(true) {

}

VROAudioPlayer::~VROAudioPlayer() {

}

void VROAudioPlayer::setTrack(NSURL *url, int loopCount) {
    if (_player) {
        [_player stop];
    }
    
    _player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:NULL];
    _player.numberOfLoops = loopCount;
    [_player prepareToPlay];
}

void VROAudioPlayer::setTrack(NSData *data, int loopCount) {
    if (_player) {
        [_player stop];
    }
    
    _player = [[AVAudioPlayer alloc] initWithData:data error:NULL];
    _player.numberOfLoops = loopCount;
    [_player prepareToPlay];
}

void VROAudioPlayer::stop() {
    doFadeThenStop();
}

void VROAudioPlayer::play() {
    _player.volume = 1.0;
    [_player play];
    _paused = false;
}

void VROAudioPlayer::pause() {
    doFadeThenPause();
}

void VROAudioPlayer::playIfPaused() {
    if (_paused) {
        [_player play];
    }
}

void VROAudioPlayer::doFadeThenPause() {
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

void VROAudioPlayer::doFadeThenStop() {
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
