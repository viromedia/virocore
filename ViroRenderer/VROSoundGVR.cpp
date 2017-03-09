//
//  VROSoundGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/26/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "VROSoundGVR.h"
#include "VROSoundDataGVR.h"
#include "VROLog.h"

std::shared_ptr<VROSoundGVR> VROSoundGVR::create(std::string path,
                                                 std::shared_ptr<gvr::AudioApi> gvrAudio,
                                                 VROSoundType type,
                                                 bool isLocalFile) {
    std::shared_ptr<VROSoundGVR> sound = std::make_shared<VROSoundGVR>(path, gvrAudio, type, isLocalFile);
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


VROSoundGVR::VROSoundGVR(std::string path,
                                 std::shared_ptr<gvr::AudioApi> gvrAudio,
                                 VROSoundType type,
                                 bool isLocalFile) :
    _gvrAudio(gvrAudio) {
    _type = type;
    std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(path, isLocalFile);
    _data = std::dynamic_pointer_cast<VROSoundData>(data);
}

VROSoundGVR::VROSoundGVR(std::shared_ptr<VROSoundData> data, std::shared_ptr<gvr::AudioApi> gvrAudio,
                         VROSoundType type) :
    _data(data),
    _gvrAudio(gvrAudio) {
    _type = type;
}

VROSoundGVR::~VROSoundGVR() {
    if (_gvrAudio && _gvrAudio->IsSoundPlaying(_audioId)) {
        _gvrAudio->PauseSound(_audioId);
    }

    if (_gvrAudio && _data) {
        _gvrAudio->UnloadSoundfile(_data->getLocalFilePath());
    }
}

void VROSoundGVR::setup() {
    _data->setDelegate(shared_from_this());
}

void VROSoundGVR::play() {
    if (!_ready) {
        pwarn("VROSoundGVR play() called before it's ready.");
        return;
    }

    _gvrAudio->Resume();

    // Check if a loaded sound has become invalid; if so, we need
    // to stop it (to destroy it), and reload it
    if (_audioId != -1 && !_gvrAudio->IsSourceIdValid(_audioId)) {
        _gvrAudio->StopSound(_audioId);
        _audioId = -1;
    }

    // create the sound if it hasn't been created yet.
    if (_audioId == -1) {
        switch (_type) {
            case VROSoundType::Normal:
                _audioId = _gvrAudio->CreateStereoSound(_data->getLocalFilePath());
                break;
            case VROSoundType::Spatial:
                _audioId = _gvrAudio->CreateSoundObject(_data->getLocalFilePath());
                break;
            case VROSoundType::SoundField:
                _audioId = _gvrAudio->CreateSoundfield(_data->getLocalFilePath());
                break;
        }

        setProperties();
        _gvrAudio->PlaySound(_audioId, _loop);
    }
    else {
        _gvrAudio->ResumeSound(_audioId);
    }

    _paused = false;
}

void VROSoundGVR::pause() {
    if (_audioId != -1) {
        _gvrAudio->PauseSound(_audioId);
        _paused = true;
    }
}

void VROSoundGVR::setVolume(float volume) {
    _volume = volume;
    if (_audioId != -1) {
        if (!_muted) {
            _gvrAudio->SetSoundVolume(_audioId, volume);
        }
    }
}

void VROSoundGVR::setMuted(bool muted) {
    _muted = muted;
    if (_audioId != -1) {
        if (muted) {
            _gvrAudio->SetSoundVolume(_audioId, 0);
        } else {
            _gvrAudio->SetSoundVolume(_audioId, _volume);
        }
    }
}

void VROSoundGVR::setLoop(bool loop) {
    if (loop == _loop) {
        return;
    }

    _loop = loop;
    if (_gvrAudio && _audioId != -1) {
        // We have to stop the sound and recreate it to change the
        // _loop setting
        _gvrAudio->StopSound(_audioId);
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
    if (_audioId != -1 && _type == VROSoundType::SoundField) {
        _gvrAudio->SetSoundfieldRotation(_audioId,
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
    if (_audioId != -1 && _type == VROSoundType::Spatial) {
        _gvrAudio->SetSoundObjectPosition(_audioId, _transformedPosition.x, _transformedPosition.y,
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
    
    if (_audioId != -1 && _type == VROSoundType::Spatial) {
        _gvrAudio->SetSoundObjectDistanceRolloffModel(_audioId, _gvrRolloffType,
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
    if (_gvrAudio) {
        bool result = _gvrAudio->PreloadSoundfile(_data->getLocalFilePath());
        passert(result);

        if (_delegate) {
            _delegate->soundIsReady();
        }
    }
}

void VROSoundGVR::setProperties() {
    if (_audioId == -1) {
        return;
    }

    _gvrAudio->SetSoundVolume(_audioId, _muted ? 0 : _volume);
    
    if (_type == VROSoundType::Spatial) {
        _gvrAudio->SetSoundObjectPosition(_audioId, _transformedPosition.x, _transformedPosition.y,
                                          _transformedPosition.z);
        _gvrAudio->SetSoundObjectDistanceRolloffModel(_audioId, _gvrRolloffType,
                                                      _rolloffMinDistance, _rolloffMaxDistance);
    } else if (_type == VROSoundType::SoundField) {
        _gvrAudio->SetSoundfieldRotation(_audioId,
                                         {_rotation.X, _rotation.Y, _rotation.Z, _rotation.W});
    }
}
