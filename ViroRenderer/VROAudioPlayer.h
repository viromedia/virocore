//
//  VROAudioPlayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROAudioPlayer_h
#define VROAudioPlayer_h

#include <string>

class VROData;

class VROAudioPlayer {
    
public:
    
    VROAudioPlayer() {}
    virtual ~VROAudioPlayer() {}
    
    virtual void setTrack(std::string url, int loopCount) = 0;
    virtual void setTrack(std::shared_ptr<VROData> data, int loopCount) = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    
};

#endif /* VROAudioPlayer_h */
