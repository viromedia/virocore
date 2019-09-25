//
//  VRORenderDelegate.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
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
