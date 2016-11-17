//
//  VROSoundEffectAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSoundEffectAndroid.h"
#include "VROLog.h"

VROSoundEffectAndroid::VROSoundEffectAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _gvrAudio(gvrAudio),
    _fileName(fileName) {

    pinfo("Pre-loading GVR sound effect [%s]", fileName.c_str());

    bool result = _gvrAudio->PreloadSoundfile(fileName);
    passert (result);
}

VROSoundEffectAndroid::~VROSoundEffectAndroid() {
    _gvrAudio->UnloadSoundfile(_fileName);
    pinfo("Unloaded GVR sound effect [%s]", _fileName.c_str());
}

void VROSoundEffectAndroid::play() {
    // We use stereo sound because sound objects work with mono-channel only
    gvr::AudioSourceId audioId = _gvrAudio->CreateStereoSound(_fileName);
    passert (audioId != -1); // kInvalidId (not in Google's headers, but should be)

    _gvrAudio->PlaySound(audioId, false);
}

