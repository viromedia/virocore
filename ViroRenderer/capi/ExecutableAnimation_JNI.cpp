//
//  ExecutableAnimation_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ExecutableAnimation_JNI.h"
#include "Node_JNI.h"
#include "LazyMaterial_JNI.h"
#include <VROPlatformUtil.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ExecutableAnimation_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeWrapNodeAnimation)(VRO_ARGS
                                             VRO_REF nodeRef,
                                             VRO_STRING jkey) {

    std::shared_ptr<VRONode> node = Node::native(nodeRef);

    std::shared_ptr<VROExecutableAnimation> animation;
    if (!VRO_IS_STRING_EMPTY(jkey)) {
        std::string key_s = VROPlatformGetString(jkey, env);
        animation = node->getAnimation(key_s, true);
    }

    if (animation) {
        return ExecutableAnimation::jptr(animation);
    }
    else {
        return 0;
    }
}

VRO_METHOD(void, nativeExecuteAnimation)(VRO_ARGS
                                         VRO_REF nativeRef, VRO_REF nodeRef) {
    // Hold a global reference to the object until the animation finishes, so that
    // we invoke its animationDidFinish callback
    VRO_OBJECT obj_g = VRO_NEW_GLOBAL_REF(obj);

    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    std::weak_ptr<VRONode> node_w = Node::native(nodeRef);

    VROPlatformDispatchAsyncRenderer([animation_w, node_w, obj_g] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            VRO_DELETE_GLOBAL_REF(obj_g);
            return;
        }
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            VRO_DELETE_GLOBAL_REF(obj_g);
            return;
        }
        animation->execute(node, [obj_g] {
            VROPlatformDispatchAsyncApplication([obj_g] {
                VROPlatformCallJavaFunction(obj_g, "animationDidFinish", "()V");
                VRO_ENV env = VROPlatformGetJNIEnv();
                VRO_DELETE_GLOBAL_REF(obj_g);
            });
        });
    });
}

VRO_METHOD(void, nativePauseAnimation)(VRO_ARGS
                                       VRO_REF nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->pause();
    });
}

VRO_METHOD(void, nativeResumeAnimation)(VRO_ARGS
                                        VRO_REF nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->resume();
    });
}

VRO_METHOD(void, nativeTerminateAnimation)(VRO_ARGS
                                           VRO_REF nativeRef, VRO_BOOL jumpToEnd) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w, jumpToEnd] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->terminate(jumpToEnd);
    });
}

VRO_METHOD(void, nativeDestroyAnimation)(VRO_ARGS
                                         VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROExecutableAnimation> *>(nativeRef);
}

} // extern "C"