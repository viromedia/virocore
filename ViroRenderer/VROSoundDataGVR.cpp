//
//  VROSoundDataGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSoundDataGVR.h"
#include <VROPlatformUtil.h>

std::shared_ptr<VROSoundDataGVR> VROSoundDataGVR::create(std::string path, bool local) {
    std::shared_ptr<VROSoundDataGVR> data = std::make_shared<VROSoundDataGVR>(path, local);
    data->setup();
    return data;
}

VROSoundDataGVR::VROSoundDataGVR(std::string path, bool local) :
_path(path),
_local(local),
_status(VROSoundDataStatus::NotLoaded) {
    // no-op
}

VROSoundDataGVR::~VROSoundDataGVR() {
    // Delete the file only if it's not a bundled resource
    if (!(_localPath == _path)) {
        VROPlatformDeleteFile(_localPath);
    }
}

void VROSoundDataGVR::setup() {
    if (_status == VROSoundDataStatus::NotLoaded) {
        std::weak_ptr<VROSoundDataGVR> weakPtr = shared_from_this();
        std::function<void(std::string)> onFinish = [weakPtr](std::string fileName) {
            std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
            if (data) {
                data->ready(fileName);
            }
        };
        if (_local) {
            loadSoundFromResource(_path, onFinish);
        } else {
            std::function<void(std::string)> onError = [weakPtr](std::string error) {
                std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
                if (data) {
                    data->error(error);
                }
            };

            loadSoundFromURL(_path, onFinish, onError);
        }
    }
}

void VROSoundDataGVR::ready(std::string fileName) {
    _status = VROSoundDataStatus::Ready;
    _error.clear();
    _localPath = fileName;

    notifyDelegateOfStatus();
}

void VROSoundDataGVR::error(std::string err) {
    _status = VROSoundDataStatus::Error;
    _error = err;

    notifyDelegateOfStatus();
}

std::string VROSoundDataGVR::getLocalFilePath() {
    return _localPath;
}

void VROSoundDataGVR::setDelegate(std::weak_ptr<VROSoundDataDelegate> delegate) {
    _delegate = delegate;
    notifyDelegateOfStatus();
}

void VROSoundDataGVR::notifyDelegateOfStatus() {
    std::shared_ptr<VROSoundDataDelegate> strongDelegate = _delegate.lock();
    if (strongDelegate) {
        if (_status == VROSoundDataStatus::Ready) {
            strongDelegate->dataIsReady();
        }
        else if (_status == VROSoundDataStatus::Error) {
            strongDelegate->dataError(_error);
        }
    }
}

void VROSoundDataGVR::loadSoundFromURL(std::string path,
                                       std::function<void(std::string)> onFinish,
                                       std::function<void(std::string)> onError) {
    VROPlatformDispatchAsyncBackground([path, onFinish, onError] {
        bool isTemp = false;
        bool success = false;
        std::string filename = VROPlatformDownloadURLToFile(path, &isTemp, &success);

        if (success) {
            onFinish(filename);
        }
        else {
            onError("Failed to load sound from URL");
        }
    });
}

void VROSoundDataGVR::loadSoundFromResource(std::string path,
                                            std::function<void(std::string)> onFinish) {
    VROPlatformDispatchAsyncBackground([path, onFinish] {
        bool isTemp = false;
        std::string filename = VROPlatformCopyResourceToFile(path);

        onFinish(filename);
    });
}
