//
//  VROAudioPlayerMacOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayerMacOS.h"
#include "VROData.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VROMath.h"

VROAudioPlayerMacOS::VROAudioPlayerMacOS(std::string url, bool isLocalUrl) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false),
    _isLocal(isLocalUrl),
    _url(url) {
        
}

VROAudioPlayerMacOS::VROAudioPlayerMacOS(std::shared_ptr<VROData> data) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false) {
    
    _player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:data->getData() length:data->getDataLength()]
                                            error:NULL];
    [_player prepareToPlay];
}

VROAudioPlayerMacOS::VROAudioPlayerMacOS(std::shared_ptr<VROSoundData> data) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false) {
        
    _data = data;
}

void VROAudioPlayerMacOS::setDelegate(std::shared_ptr<VROSoundDelegateInternal> delegate) {
    _delegate = delegate;
}

void VROAudioPlayerMacOS::setup() {
    if (_data) {
        _data->setDelegate(shared_from_this());
    }
    else if (!_url.empty()) {
        if (_isLocal) {
            NSURL *localUrlObj =[NSURL fileURLWithPath:[NSString stringWithUTF8String:_url.c_str()]];
            _player = [[AVAudioPlayer alloc] initWithContentsOfURL:localUrlObj error:NULL];
            [_player prepareToPlay];
        }
        else {
            // Need shared pointer to prevent this object from deleted underneath us
            std::shared_ptr<VROAudioPlayerMacOS> shared = shared_from_this();
            
            NSURL *urlObj = [NSURL URLWithString:[NSString stringWithUTF8String:_url.c_str()]];
            VROPlatformDownloadDataWithURL(urlObj, ^(NSData *data, NSError *error) {
                if (data && !error) {
                    shared->_player = [[AVAudioPlayer alloc] initWithData:data error:NULL];
                    shared->updatePlayerProperties();
                    
                    shared->_audioDelegate = [[VROAudioPlayerDelegate alloc] initWithSoundDelegate:_delegate];
                    shared->_player.delegate = _audioDelegate;
                    shared->_delegate->soundIsReady();
                }
                else {
                    shared->_delegate->soundDidFail("Failed to load sound");
                }
            });
        }
    }
    
    if(_isLocal) {
        _delegate->soundIsReady();
    }
}

VROAudioPlayerMacOS::~VROAudioPlayerMacOS() {
    if (_player) {
        [_player stop];
    }
}

void VROAudioPlayerMacOS::updatePlayerProperties() {
    _player.numberOfLoops = _loop ? -1 : 0;
    _player.volume = _muted ? 0 : _playVolume;
}

void VROAudioPlayerMacOS::setLoop(bool loop) {
    if (_loop == loop) {
        return;
    }
    
    _loop = loop;
    if (_player) {
        _player.numberOfLoops = loop ? -1 : 0;
        
        // If we were not explicitly paused and loop was activated,
        // play the sound (so it turns back on)
        if (!_paused && _loop) {
            [_player play];
        }
    }
}

void VROAudioPlayerMacOS::setMuted(bool muted) {
    _muted = muted;
    if (_player) {
        _player.volume = muted ? 0 : _playVolume;
    }
}

void VROAudioPlayerMacOS::seekToTime(float seconds) {
    if (_player) {
        seconds = clamp(seconds, 0, _player.duration);
        _player.currentTime = seconds;
    }
}

void VROAudioPlayerMacOS::setVolume(float volume) {
    _playVolume = volume;
    if (_player) {
        _player.volume = volume;
    }
}

void VROAudioPlayerMacOS::play() {
    _paused = false;
    if (_player) {
        _player.volume = _playVolume;
        [_player play];
    }
}

void VROAudioPlayerMacOS::pause() {
    _paused = true;
    if (_player) {
        doFadeThenPause();
    }
}

void VROAudioPlayerMacOS::doFadeThenPause() {
    // Grab the shared_ptr to this, so that we retain this in the dispatch_after;
    // otherwise only 'this' is captured, so the ref-count can slip to zero and
    // cause a crash
    std::shared_ptr<VROAudioPlayerMacOS> capturedSelf = shared_from_this();
    
    if (_player.volume > 0.1) {
        _player.volume = _player.volume - 0.1;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            capturedSelf->doFadeThenPause();
        });
    }
    else {
        [_player pause];
    }
}

#pragma mark VROSoundDataDelegate Implementation

void VROAudioPlayerMacOS::dataIsReady() {
    if (!_player) {
        _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:_data->getLocalFilePath().c_str()]]
                                                         error:NULL];
        [_player prepareToPlay];
        updatePlayerProperties();
        
        if (!_paused) {
            [_player play];
        }
    }
}

void VROAudioPlayerMacOS::dataError(std::string error) {
    if (_delegate) {
        _delegate->soundDidFail(error);
    }
}

#pragma mark - VROAudioPlayerDelegate implementation

@implementation VROAudioPlayerDelegate {
    std::weak_ptr<VROSoundDelegateInternal> _delegate;
}

- (id)initWithSoundDelegate:(std::shared_ptr<VROSoundDelegateInternal>)soundDelegate {
    self = [super init];
    if (self) {
        _delegate = soundDelegate;
    }
    return self;
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag {
    std::shared_ptr<VROSoundDelegateInternal> soundDelegate = _delegate.lock();
    if (soundDelegate) {
        soundDelegate->soundDidFinish();
    }
}

@end
