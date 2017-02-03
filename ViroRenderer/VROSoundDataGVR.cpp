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
_path(path) {
    if (local) {
        _localPath = _path;
    }
}

VROSoundDataGVR::~VROSoundDataGVR() {
    // Delete the file only if it's not a bundled resource
    if (!(_localPath == _path)) {
        VROPlatformDeleteFile(_localPath);
    }
}

void VROSoundDataGVR::setup() {
    // If we aren't read and the file isn't local, then load the sound
    if (_path != _localPath && !_ready) {
        std::weak_ptr<VROSoundDataGVR> weakPtr = shared_from_this();

        loadSound(_path, [weakPtr](std::string fileName) {
            std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
            if (data) {
                data->ready(fileName);
            }
        });
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

void VROSoundDataGVR::loadSound(std::string path, std::function<void(std::string)> onFinish) {
    VROPlatformDispatchAsyncBackground([path, onFinish] {
        bool isTemp = false;
        std::string filename = VROPlatformDownloadURLToFile(path, &isTemp);

        onFinish(filename);
    });
}
