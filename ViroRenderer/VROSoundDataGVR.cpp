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
_local(local) {
    // no-op
}

VROSoundDataGVR::~VROSoundDataGVR() {
    // Delete the file only if it's not a bundled resource
    if (!(_localPath == _path)) {
        VROPlatformDeleteFile(_localPath);
    }
}

void VROSoundDataGVR::setup() {
    // If we aren't read and the file isn't local, then load the sound
    if (!_ready) {
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
            loadSoundFromURL(_path, onFinish);
        }
    }
}

void VROSoundDataGVR::ready(std::string fileName) {
    _ready = true;
    _localPath = fileName;
    notifyDelegateIfReady();
}

std::string VROSoundDataGVR::getLocalFilePath() {
    return _localPath;
}

void VROSoundDataGVR::setDelegate(std::weak_ptr<VROSoundDataDelegate> delegate) {
    _delegate = delegate;
    notifyDelegateIfReady();
}

void VROSoundDataGVR::notifyDelegateIfReady() {
    std::shared_ptr<VROSoundDataDelegate> strongDelegate = _delegate.lock();
    if (strongDelegate && _ready) {
        strongDelegate->dataIsReady();
    }
}

void VROSoundDataGVR::loadSoundFromURL(std::string path,
                                       std::function<void(std::string)> onFinish) {
    VROPlatformDispatchAsyncBackground([path, onFinish] {
        bool isTemp = false;
        std::string filename = VROPlatformDownloadURLToFile(path, &isTemp);

        onFinish(filename);
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
