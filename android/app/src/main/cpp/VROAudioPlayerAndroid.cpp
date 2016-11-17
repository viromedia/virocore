//
//  VROAudioPlayerAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayerAndroid.h"
#include "VROLog.h"

VROAudioPlayerAndroid::VROAudioPlayerAndroid(std::string fileName, std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _gvrAudio(gvrAudio),
    _fileName(fileName),
    _loop(false),
    _volume(1.0) {

    pinfo("Pre-loading GVR sound effect [%s]", fileName.c_str());

    bool result = _gvrAudio->PreloadSoundfile(fileName);
    passert (result);
}

VROAudioPlayerAndroid::~VROAudioPlayerAndroid() {
    _gvrAudio->StopSound(_audioId);
    _gvrAudio->UnloadSoundfile(_fileName);

    pinfo("Unloaded GVR audio player [%s]", _fileName.c_str());
}

void VROAudioPlayerAndroid::setLoop(bool loop) {
    _loop = loop;

    if (_gvrAudio->IsSoundPlaying(_audioId)) {
        _gvrAudio->StopSound(_audioId);
        play();
    }
}

void VROAudioPlayerAndroid::play() {
    /*
     Filed https://github.com/googlevr/gvr-android-sdk/issues/294: IsSoundPlaying is broken, and ambiguous
     Until resolution, pause() and play() will just restart the sound from the beginning.
     */
    _audioId = _gvrAudio->CreateStereoSound(_fileName);
    passert (_audioId != -1); // kInvalidId (not in Google's headers, but should be)

    _gvrAudio->SetSoundVolume(_audioId, _volume);
    _gvrAudio->PlaySound(_audioId, _loop);
}

void VROAudioPlayerAndroid::pause() {
    _gvrAudio->StopSound(_audioId);
}

void VROAudioPlayerAndroid::setVolume(float volume) {
    _volume = volume;
    if (_gvrAudio->IsSoundPlaying(_audioId)) {
        _gvrAudio->SetSoundVolume(_audioId, _volume);
    }
}