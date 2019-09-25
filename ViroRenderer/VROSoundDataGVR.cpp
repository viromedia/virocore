//
//  VROSoundDataGVR.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 1/30/17.
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

#include "VROSoundDataGVR.h"
#include "VROPlatformUtil.h"
#include "VROLog.h"

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
                data->ready(fileName, isTemp);
            }
        };
 
        std::function<void(std::string)> onError = [weakPtr](std::string error) {
            std::shared_ptr<VROSoundDataGVR> data = weakPtr.lock();
            if (data) {
                data->error(error);
            }
        };
        VROModelIOUtil::retrieveResourceAsync(resource, resourceType, onFinish,
               [resource, onError]() {
                   // Error callback
                   pinfo("Failed to load sound resource [%s]", resource.c_str());
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
