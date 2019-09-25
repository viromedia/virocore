//
//  ViroContext_JNI.cpp
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

#include <memory>
#include "ViroContext_JNI.h"
#include "VROPlatformUtil.h"
#include "VROVector3f.h"
#include "VROCamera.h"

#if VRO_PLATFORM_ANDROID
#include "VRORenderer_JNI.h"
#include "ViroContextAndroid_JNI.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ViroContext_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ViroContext_##method_name
#endif

extern "C" {

#if VRO_PLATFORM_ANDROID
    // TODO wasm
VRO_METHOD(VRO_REF(ViroContext), nativeCreateViroContext)(VRO_ARGS
                                                          VRO_REF(VROSceneRenderer) renderer_j) {

    std::shared_ptr<ViroContext> context;
#if VRO_PLATFORM_ANDROID
    context = std::make_shared<ViroContextAndroid>(VRO_REF_GET(VROSceneRenderer, renderer_j));
#else
    // TODO wasm
#endif
    return VRO_REF_NEW(ViroContext, context);
}
#endif

VRO_METHOD(void, nativeDeleteViroContext)(VRO_ARGS
                                          VRO_REF(ViroContext) context_j) {
    VRO_REF_DELETE(ViroContext, context_j);
}

VRO_METHOD(void, nativeGetCameraOrientation)(VRO_ARGS
                                             VRO_REF(ViroContext) context_j,
                                             VRO_OBJECT callback) {
    VRO_WEAK weakCallback = VRO_NEW_WEAK_GLOBAL_REF(callback);
    std::weak_ptr<ViroContext> context_w = VRO_REF_GET(ViroContext, context_j);

    VROPlatformDispatchAsyncRenderer([context_w, weakCallback] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT jCallback = VRO_NEW_LOCAL_REF(weakCallback);
        if (VRO_IS_OBJECT_NULL(jCallback)) {
            return;
        }
        std::shared_ptr<ViroContext> helperContext = context_w.lock();
        if (!helperContext) {
            return;
        }

        const VROCamera &camera = helperContext->getCamera();
        VROVector3f position = camera.getPosition();
        VROVector3f rotation = camera.getRotation().toEuler();
        VROVector3f forward = camera.getForward();
        VROVector3f up = camera.getUp();

        VROPlatformCallHostFunction(jCallback,
                                    "onGetCameraOrientation", "(FFFFFFFFFFFF)V",
                                    position.x, position.y, position.z,
                                    rotation.x, rotation.y, rotation.z,
                                    forward.x, forward.y, forward.z,
                                    up.x, up.y, up.z);
        VRO_DELETE_LOCAL_REF(jCallback);
    });
}

}  // extern "C"
