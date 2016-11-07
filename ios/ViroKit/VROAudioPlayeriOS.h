//
//  VROAudioPlayeriOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROAudioPlayeriOS_h
#define VROAudioPlayeriOS_h

#include "VROAudioPlayer.h"
#include <AVFoundation/AVFoundation.h>

class VROAudioPlayeriOS : public VROAudioPlayer {
    
public:
    
    VROAudioPlayeriOS();
    virtual ~VROAudioPlayeriOS();
    
    void setTrack(std::string url, int loopCount);
    void setTrack(std::shared_ptr<VROData> data, int loopCount);
    void play();
    void stop();
    void pause();
    
private:
    
    AVAudioPlayer *_player;
    
    void doFadeThenPause();
    void doFadeThenStop();
    
};

#endif /* VROAudioPlayeriOS_h */
