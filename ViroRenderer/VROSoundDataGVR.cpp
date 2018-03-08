//
//  VROSoundDataGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSoundDataGVR.h"
#include <VROPlatformUtil.h>

std::shared_ptr<VROSoundDataGVR> VROSoundDataGVR::create(std::string resource, VROResourceType type) {
    std::shared_ptr<VROSoundDataGVR> data = std::make_shared<VROSoundDataGVR>(resource, type);
    data->setup(resource, type);
    return data;
}

VROSoundDataGVR::VROSoundDataGVR(std::string resource, VROResourceType type) :
    _resource(resource),
    _resourceType(type),
    _status(VROSoundDataStatus::NotLoaded),
    _deleteFileOnDestroy(false) {
    
}

VROSoundDataGVR::~VROSoundDataGVR() {
    if (_deleteFileOnDestroy) {
        VROPlatformDeleteFile(_localPath);
    }
}

void VROSoundDataGVR::setup(std::string resource, VROResourceType resourceType) {
    if (_status == VROSoundDataStatus::NotLoaded) {
        std::weak_ptr<VROSoundDataGVR> weakPtr = shared_from_this();
        std::function<void(std::string, bool)> onFinish = [weakPtr](std::string fileName, bool isTemp) {
            std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
            if (data) {
                // TODO Is this safe to call from a background thread?
                data->ready(fileName, isTemp);
            }
        };
 
        std::function<void(std::string)> onError = [weakPtr](std::string error) {
            std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
            if (data) {
                data->error(error);
            }
        };
        
        VROModelIOUtil::retrieveResourceAsync(resource, resourceType, onFinish, [onError]() {
            onError("Failed to load sound");
        });
    }
}

void VROSoundDataGVR::ready(std::string fileName, bool isTemp) {
    _status = VROSoundDataStatus::Ready;
    _error.clear();
    _localPath = fileName;
    _deleteFileOnDestroy = isTemp;

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
