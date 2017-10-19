//
//  VRORenderToTextureDelegateAndroid.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRO_RENDER_TEXTURE_DELEGATE_ANDROID_H
#define VRO_RENDER_TEXTURE_DELEGATE_ANDROID_H

#include <memory>
#include "VRORenderToTextureDelegate.h"
#include "VROAVRecorderAndroid.h"

class VROAVRecorderAndroid;
class VRORenderTarget;
class VRODriver;

class VRORenderToTextureDelegateAndroid : public VRORenderToTextureDelegate {
public:
    VRORenderToTextureDelegateAndroid(std::shared_ptr<VROAVRecorderAndroid> recorder) {
        _w_recorder = recorder;
    };

    virtual ~VRORenderToTextureDelegateAndroid() {};

    void renderedFrameTexture(std::shared_ptr<VRORenderTarget> input, std::shared_ptr<VRODriver> driver) {
        std::shared_ptr<VROAVRecorderAndroid> recorder = _w_recorder.lock();
        if(recorder) {
            recorder->onRenderedFrameTexture(input, driver);
        }
    }

private:
    std::weak_ptr<VROAVRecorderAndroid> _w_recorder;
};

#endif //VRO_RENDER_TEXTURE_DELEGATE_ANDROID_H
