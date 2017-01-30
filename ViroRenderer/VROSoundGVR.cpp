//
//  VROSoundGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/26/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "VROSoundGVR.h"
#include <VROPlatformUtil.h>
#include "VROLog.h"
#include "VROStringUtil.h"

std::map<std::string, std::string> VROSoundGVR::_preloadedFiles = {};

VROSoundGVR::VROSoundGVR(std::string path,
                                 std::shared_ptr<gvr::AudioApi> gvrAudio,
                                 VROSoundType type,
                                 bool isLocalFile) :
    _gvrAudio(gvrAudio),
    _originalPath(path) {
    _type = type;
    if (isLocalFile) {
        _fileName = path;
        pinfo("Pre-loading GVR sound effect, local file: [%s]", _fileName.c_str());
        bool result = _gvrAudio->PreloadSoundfile(_fileName);
        passert (result);

        _isReady = true;
        _delegate->soundIsReady();
    } else {
        // Check if we've already downloaded & preloaded the file
        auto it = _preloadedFiles.find(path);
        if (it == _preloadedFiles.end()) {
            loadSound(path, [this, path](std::string filename) {
                _fileName = filename;

                pinfo("Pre-loading GVR sound effect, temp file: [%s]", _fileName.c_str());

                bool result = _gvrAudio->PreloadSoundfile(_fileName);
                passert(result);

                _preloadedFiles[path] = filename;

                _isReady = true;
                if (_delegate != nullptr) {
                    _delegate->soundIsReady();
                }
            });
        } else {
            _fileName = it->second;
            _isReady = true;
            _delegate->soundIsReady();
        }
    }
}

VROSoundGVR::~VROSoundGVR() {
    if (_audioId) {
        _gvrAudio->PauseSound(_audioId);
    }
    auto it = _preloadedFiles.find(_originalPath);
    if (it != _preloadedFiles.end()) {
        _preloadedFiles.erase(it);
    }
    _gvrAudio->UnloadSoundfile(_fileName);

    // If originalPath is the same as fileName, then we know the file was a local resource, so don't
    // delete it. If they differ, then delete the local file.
    // TODO: VIRO-749 don't delete preloaded files either
    if (!VROStringUtil::strcmpinsensitive(_originalPath.c_str(), _fileName.c_str())) {
        VROPlatformDeleteFile(_fileName);
    }
    pinfo("Unloaded GVR sound effect [%s]", _fileName.c_str());
}

void VROSoundGVR::play() {
    // create the sound if it hasn't been created yet.
    if (_audioId == -1) {
        switch (_type) {
            case VROSoundType::Normal:
                _audioId = _gvrAudio->CreateStereoSound(_fileName);
                break;
            case VROSoundType::Spatial:
                _audioId = _gvrAudio->CreateSoundObject(_fileName);
                break;
            case VROSoundType::SoundField:
                _audioId = _gvrAudio->CreateSoundfield(_fileName);
                break;
        }
    }
    passert (_audioId != -1); // kInvalidId (not in Google's headers, but should be)
    // Set properties before we start playing because we *just* created the GVR Sound objects
    setProperties();

    _gvrAudio->Resume();
    _gvrAudio->PlaySound(_audioId, _loop);
}

void VROSoundGVR::pause() {
    if (_audioId != -1) {
        _gvrAudio->PauseSound(_audioId);
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
    _loop = loop;
    if (_gvrAudio && _audioId != -1) {
        _gvrAudio->PlaySound(_audioId, _loop);
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

void VROSoundGVR::loadSound(std::string path, std::function<void(std::string)> onFinish) {
    VROPlatformDispatchAsyncBackground([path, onFinish] {
        bool isTemp = false;
        std::string filename = VROPlatformDownloadURLToFile(path, &isTemp);
        
        onFinish(filename);
    });
}
