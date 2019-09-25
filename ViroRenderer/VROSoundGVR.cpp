//
//  VROSoundGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/26/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "VROSoundGVR.h"
#include "VROSoundDataGVR.h"
#include "VROLog.h"
#include "vr/gvr/capi/include/gvr_audio.h"

std::shared_ptr<VROSoundGVR> VROSoundGVR::create(std::string resource, VROResourceType resourceType,
                                                 std::shared_ptr<gvr::AudioApi> gvrAudio,
                                                 VROSoundType type) {
    std::shared_ptr<VROSoundGVR> sound = std::make_shared<VROSoundGVR>(resource, resourceType, gvrAudio,
                                                                       type);
    sound->setup();
    return sound;
}

std::shared_ptr<VROSoundGVR> VROSoundGVR::create(std::shared_ptr<VROSoundData> data,
                                                 std::shared_ptr<gvr::AudioApi> gvrAudio,
                                                 VROSoundType type) {
    std::shared_ptr<VROSoundGVR> sound = std::make_shared<VROSoundGVR>(data, gvrAudio, type);
    sound->setup();
    return sound;
}

VROSoundGVR::VROSoundGVR(std::string resource, VROResourceType resourceType,
                         std::shared_ptr<gvr::AudioApi> gvrAudio,
                         VROSoundType type) :
    _gvrAudio(gvrAudio) {
    _type = type;
    std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(resource, resourceType);
    _data = std::dynamic_pointer_cast<VROSoundData>(data);
    _gvrRolloffType = GVR_AUDIO_ROLLOFF_NONE;
}

VROSoundGVR::VROSoundGVR(std::shared_ptr<VROSoundData> data, std::shared_ptr<gvr::AudioApi> gvrAudio,
                         VROSoundType type) :
    _data(data),
    _gvrAudio(gvrAudio) {
    _type = type;
    _gvrRolloffType = GVR_AUDIO_ROLLOFF_NONE;
}

VROSoundGVR::~VROSoundGVR() {
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (!gvrAudio) {
        return;
    }
    
    if (gvrAudio && gvrAudio->IsSoundPlaying(_audioId)) {
        gvrAudio->PauseSound(_audioId);
    }
    if (gvrAudio && _data) {
        gvrAudio->UnloadSoundfile(_data->getLocalFilePath());
    }
}

void VROSoundGVR::setup() {
    _data->setDelegate(shared_from_this());
    if (_ready) {
        _gvrAudio.lock()->PreloadSoundfile(_data->getLocalFilePath());
    }
}

void VROSoundGVR::play() {
    if (!_ready) {
        pwarn("VROSoundGVR play() called before it's ready.");
        return;
    }
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (!gvrAudio) {
        return;
    }

    gvrAudio->Resume();

    // Check if a loaded sound has become invalid; if so, we need
    // to stop it (to destroy it), and reload it
    if (_audioId != -1 && !gvrAudio->IsSourceIdValid(_audioId)) {
        gvrAudio->StopSound(_audioId);
        _audioId = -1;
    }

    // create the sound if it hasn't been created yet.
    if (_audioId == -1) {

        // For some reason GVR behaves differently between iOS and Android where on iOS it does not
        // like the path starting with "file://" whereas their Android API does want it, so remove the
        // "file://" prefix only on iOS
#if VRO_PLATFORM_ANDROID
        std::string path = _data->getLocalFilePath();
#else
        std::string path = _data->getLocalFilePath().substr(7);
#endif
        switch (_type) {
            case VROSoundType::Normal:
                _audioId = gvrAudio->CreateStereoSound(path);
                break;
            case VROSoundType::Spatial:
                _audioId = gvrAudio->CreateSoundObject(path);
                break;
            case VROSoundType::SoundField:
                _audioId = gvrAudio->CreateSoundfield(path);
                break;
        }

        setProperties();
        gvrAudio->PlaySound(_audioId, _loop);
    }
    else {
        gvrAudio->ResumeSound(_audioId);
    }

    _paused = false;
}

void VROSoundGVR::pause() {
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (!gvrAudio) {
        return;
    }
    if (_audioId != -1) {
        gvrAudio->PauseSound(_audioId);
        _paused = true;
    }
}

void VROSoundGVR::setVolume(float volume) {
    _volume = volume;
    
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1) {
        if (!_muted) {
            gvrAudio->SetSoundVolume(_audioId, volume);
        }
    }
}

void VROSoundGVR::setMuted(bool muted) {
    _muted = muted;
    
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1) {
        if (muted) {
            gvrAudio->SetSoundVolume(_audioId, 0);
        } else {
            gvrAudio->SetSoundVolume(_audioId, _volume);
        }
    }
}

void VROSoundGVR::setLoop(bool loop) {
    if (loop == _loop) {
        return;
    }

    _loop = loop;
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1) {
        // We have to stop the sound and recreate it to change the
        // _loop setting
        gvrAudio->StopSound(_audioId);
        if (!_paused) {
            play();
        }
    }
}

void VROSoundGVR::seekToTime(float seconds) {
    // no-op: GVR doesn't support this yet
}

void VROSoundGVR::setRotation(VROQuaternion rotation) {
    _rotation = rotation;
    
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1 && _type == VROSoundType::SoundField) {
        gvrAudio->SetSoundfieldRotation(_audioId,
                                         {rotation.X, rotation.Y, rotation.Z, rotation.W});
    }
}

void VROSoundGVR::setPosition(VROVector3f position) {
    _position = position;
}

VROVector3f VROSoundGVR::getPosition() {
    return _position;
}

void VROSoundGVR::setTransformedPosition(VROVector3f transformedPosition) {
    _transformedPosition = transformedPosition;
    
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1 && _type == VROSoundType::Spatial) {
        gvrAudio->SetSoundObjectPosition(_audioId, _transformedPosition.x, _transformedPosition.y,
                                          _transformedPosition.z);
    }
}

void VROSoundGVR::setDistanceRolloffModel(VROSoundRolloffModel model, float minDistance,
                                              float maxDistance) {
    _rolloffModel = model;
    _rolloffMinDistance = minDistance;
    _rolloffMaxDistance = maxDistance;
    
    switch(model) {
        case VROSoundRolloffModel::Linear:
            _gvrRolloffType = GVR_AUDIO_ROLLOFF_LINEAR;
            break;
        case VROSoundRolloffModel::Logarithmic:
            _gvrRolloffType = GVR_AUDIO_ROLLOFF_LOGARITHMIC;
            break;
        case VROSoundRolloffModel::None:
            _gvrRolloffType = GVR_AUDIO_ROLLOFF_NONE;
            break;
    }
    
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (gvrAudio && _audioId != -1 && _type == VROSoundType::Spatial) {
        gvrAudio->SetSoundObjectDistanceRolloffModel(_audioId, (gvr_audio_distance_rolloff_type) _gvrRolloffType,
                                                      _rolloffMinDistance, _rolloffMaxDistance);
    }
}

void VROSoundGVR::setDelegate(std::shared_ptr<VROSoundDelegateInternal> delegate) {
    _delegate = delegate;
    if (_ready && _delegate) {
        _delegate->soundIsReady();
    }
}

void VROSoundGVR::dataIsReady() {
    _ready = true;
    if (_delegate) {
        _delegate->soundIsReady();
    }
}

void VROSoundGVR::dataError(std::string error) {
    if (_delegate) {
        _delegate->soundDidFail(error);
    }
}

void VROSoundGVR::setProperties() {
    if (_audioId == -1) {
        return;
    }
    std::shared_ptr<gvr::AudioApi> gvrAudio = _gvrAudio.lock();
    if (!gvrAudio) {
        return;
    }

    gvrAudio->SetSoundVolume(_audioId, _muted ? 0 : _volume);
    
    if (_type == VROSoundType::Spatial) {
        gvrAudio->SetSoundObjectPosition(_audioId, _transformedPosition.x, _transformedPosition.y,
                                          _transformedPosition.z);
        gvrAudio->SetSoundObjectDistanceRolloffModel(_audioId, (gvr_audio_distance_rolloff_type) _gvrRolloffType,
                                                      _rolloffMinDistance, _rolloffMaxDistance);
    } else if (_type == VROSoundType::SoundField) {
        gvrAudio->SetSoundfieldRotation(_audioId,
                                         {_rotation.X, _rotation.Y, _rotation.Z, _rotation.W});
    }
}
