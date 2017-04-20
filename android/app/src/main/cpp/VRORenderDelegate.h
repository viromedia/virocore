//
//  VRORenderDelegate.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//
#ifndef ANDROID_VRORENDERDELEGATE_H
#define ANDROID_VRORENDERDELEGATE_H

#include "VRORenderDelegateInternal.h"

class VRORenderDelegate : public VRORenderDelegateInternal {

public:

    VRORenderDelegate() {}
    virtual ~VRORenderDelegate() {}

    virtual void setupRendererWithDriver(std::shared_ptr<VRODriver> driver) {}
    virtual void renderViewDidChangeSize(float width, float height, VRORenderContext *context) {}
    virtual void willRenderEye(VROEyeType eye, const VRORenderContext *context) {}
    virtual void didRenderEye(VROEyeType eye, const VRORenderContext *context) {}

    virtual void userDidRequestExitVR() {}

private:

};

#endif //ANDROID_VRORENDERDELEGATE_H
