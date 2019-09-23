//
//  ARSceneController_JNI.h
//  ViroRenderer
//
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

#ifndef ARSceneController_JNI_h
#define ARSceneController_JNI_h

#include <memory>
#include <VROARSceneController.h>
#include <VROARDeclarativeSession.h>
#include <VROARImperativeSession.h>
#include <VROPlatformUtil.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class ARDeclarativeSceneDelegate : public VROARSceneDelegate, public VROARDeclarativeSessionDelegate {
public:
    ARDeclarativeSceneDelegate(VRO_OBJECT arSceneJavaObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arSceneJavaObject)) {
    }

    virtual ~ARDeclarativeSceneDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float intensity, VROVector3f color);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);

private:
    VRO_OBJECT _javaObject;
};

class ARImperativeSceneDelegate : public VROARSceneDelegate, public VROARImperativeSessionDelegate {
public:
    ARImperativeSceneDelegate(VRO_OBJECT arSceneJavaObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arSceneJavaObject))    {
    }

    virtual ~ARImperativeSceneDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float intensity, VROVector3f color);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);

private:
    VRO_OBJECT _javaObject;
};

#endif