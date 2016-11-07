//
//  VROSoundEffectiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSoundEffectiOS.h"
#include "VROData.h"

VROSoundEffectiOS::VROSoundEffectiOS(std::string url) {
    _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]
                                                     error:NULL];
    _player.numberOfLoops = 0;
    [_player prepareToPlay];
}

VROSoundEffectiOS::VROSoundEffectiOS(std::shared_ptr<VROData> data) {
    _player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:data->getData() length:data->getDataLength()]
                                            error:NULL];
    _player.numberOfLoops = 0;
    [_player prepareToPlay];
}

VROSoundEffectiOS::~VROSoundEffectiOS() {
    
}

void VROSoundEffectiOS::play() {
    [_player play];
}
