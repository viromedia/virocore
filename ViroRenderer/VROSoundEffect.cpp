//
//  VROSoundEffect.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/23/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSoundEffect.h"
#include "VROLog.h"

VROSoundEffect::VROSoundEffect(NSURL *url) {
    _player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:NULL];
    _player.numberOfLoops = 0;
    [_player prepareToPlay];
}

VROSoundEffect::VROSoundEffect(NSData *data) {
    _player = [[AVAudioPlayer alloc] initWithData:data error:NULL];
    _player.numberOfLoops = 0;
    [_player prepareToPlay];
}

VROSoundEffect::~VROSoundEffect() {

}

void VROSoundEffect::play() {
    [_player play];
}
