//
//  VROSoundEffectAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROSOUNDEFFECTANDROID_H
#define ANDROID_VROSOUNDEFFECTANDROID_H

#include "VROSoundEffect.h"
#include "vr/gvr/capi/include/gvr_audio.h"

class VROSoundEffectAndroid : public VROSoundEffect {

public:

    /*
     Construct a new sound effect from the given file. If given a relative
     path, the file can refer directly to an asset in the /assets directory.
     The file may also be an absolute path.
     */
    VROSoundEffectAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROSoundEffectAndroid();

    virtual void play();

private:

    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::string _fileName;

};


#endif //ANDROID_VROSOUNDEFFECTANDROID_H
