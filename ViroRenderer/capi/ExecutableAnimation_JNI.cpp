//
//  ExecutableAnimation_JNI.cpp
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

#include "ExecutableAnimation_JNI.h"
#include "Node_JNI.h"
#include "LazyMaterial_JNI.h"
#include <VROPlatformUtil.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ExecutableAnimation_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ExecutableAnimation_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROExecutableAnimation), nativeWrapNodeAnimation)(VRO_ARGS
                                                                     VRO_REF(VRONode) nodeRef,
                                                                     VRO_STRING jkey) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, nodeRef);

    std::shared_ptr<VROExecutableAnimation> animation;
    if (!VRO_IS_STRING_EMPTY(jkey)) {
        std::string key_s = VRO_STRING_STL(jkey);
        animation = node->getAnimation(key_s, true)->copy();
    }

    if (animation) {
        return VRO_REF_NEW(VROExecutableAnimation, animation);
    } else {
        return 0;
    }
}

VRO_METHOD(void, nativeExecuteAnimation)(VRO_ARGS
                                         VRO_REF(VROExecutableAnimation) nativeRef,
                                         VRO_REF(VRONode) nodeRef) {
    VRO_METHOD_PREAMBLE;

    // Hold a global reference to the object until the animation finishes, so that
    // we invoke its animationDidFinish callback
    VRO_WEAK obj_w = VRO_NEW_WEAK_GLOBAL_REF(obj);

    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, nodeRef);

    VROPlatformDispatchAsyncRenderer([animation_w, node_w, obj_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }
        animation->execute(node, [obj_w] {
            VROPlatformDispatchAsyncApplication([obj_w] {
                VRO_ENV env = VROPlatformGetJNIEnv();

                VRO_OBJECT obj_s = VRO_NEW_LOCAL_REF(obj_w);
                if (VRO_IS_OBJECT_NULL(obj_s)) {
                    VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                    return;
                }

                VROPlatformCallHostFunction(obj_s, "animationDidFinish", "()V");
                VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                VRO_DELETE_LOCAL_REF(obj_s);
            });
        });
    });
}

VRO_METHOD(void, nativePauseAnimation)(VRO_ARGS
                                       VRO_REF(VROExecutableAnimation) nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->pause();
    });
}

VRO_METHOD(void, nativeResumeAnimation)(VRO_ARGS
                                        VRO_REF(VROExecutableAnimation) nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->resume();
    });
}

VRO_METHOD(void, nativeTerminateAnimation)(VRO_ARGS
                                           VRO_REF(VROExecutableAnimation) nativeRef,
                                           VRO_BOOL jumpToEnd) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w, jumpToEnd] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->terminate(jumpToEnd);
    });
}

VRO_METHOD(void, nativeDestroyAnimation)(VRO_ARGS
                                         VRO_REF(VROExecutableAnimation) nativeRef) {
    VRO_REF_DELETE(VROExecutableAnimation, nativeRef);
}

VRO_METHOD(void, nativeSetDuration)(VRO_ARGS
                                    VRO_REF(VROExecutableAnimation) nativeRef,
                                    VRO_FLOAT durationSeconds) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w, durationSeconds] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->setDuration(durationSeconds);
    });
}

VRO_METHOD(void, nativeSetTimeOffset)(VRO_ARGS VRO_REF(VROExecutableAnimation) nativeRef, VRO_FLOAT timeOffset) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w, timeOffset] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->setTimeOffset(timeOffset);
    });
}

VRO_METHOD(void, nativeSetSpeed)(VRO_ARGS VRO_REF(VROExecutableAnimation) nativeRef, VRO_FLOAT speed) {
    std::weak_ptr<VROExecutableAnimation> animation_w = VRO_REF_GET(VROExecutableAnimation, nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w, speed] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->setSpeed(speed);
    });
}

// This should only be invoked on initialization to grab the initial duration
VRO_METHOD(VRO_FLOAT, nativeGetDuration)(VRO_ARGS
                                         VRO_REF(VROExecutableAnimation) nativeRef) {
    return VRO_REF_GET(VROExecutableAnimation, nativeRef)->getDuration();
}

} // extern "C"