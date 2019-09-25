//
//  ViroContext_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
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

#ifndef ANDROID_VIRO_CONTEXT_JNI_H
#define ANDROID_VIRO_CONTEXT_JNI_H

#include <memory>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROCamera;
class VRODriver;
class VROFrameSynchronizer;
class VROInputControllerBase;

/**
 * Context for accessing render specific information, without exposing the entire renderer.
 */
class ViroContext {
public:

    ViroContext() {}
    virtual ~ViroContext(){}

    virtual const VROCamera &getCamera() = 0;
    virtual std::shared_ptr<VRODriver> getDriver() = 0;
    virtual std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() = 0;
    virtual std::shared_ptr<VROInputControllerBase> getInputController() = 0;

};

#endif
