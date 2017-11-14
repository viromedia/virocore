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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ExecutableAnimation_##method_name

extern "C" {

JNI_METHOD(jlong, nativeWrapNodeAnimation)(JNIEnv *env,
                                             jobject obj,
                                             jlong nodeRef,
                                             jstring jkey) {

    std::shared_ptr<VRONode> node = Node::native(nodeRef);

    std::shared_ptr<VROExecutableAnimation> animation;
    if (jkey != NULL) {
        const char *key_c = env->GetStringUTFChars(jkey, NULL);
        std::string key_s(key_c);
        animation = node->getAnimation(key_s, true);
        env->ReleaseStringUTFChars(jkey, key_c);
    }

    if (animation) {
        return ExecutableAnimation::jptr(animation);
    }
    else {
        return 0;
    }
}

JNI_METHOD(void, nativeExecuteAnimation)(JNIEnv *env, jobject obj, jlong nativeRef, jlong nodeRef) {
    // Hold a global reference to the object until the animation finishes, so that
    // we invoke its animationDidFinish callback
    jweak obj_g = env->NewWeakGlobalRef(obj);

    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    std::weak_ptr<VRONode> node_w = Node::native(nodeRef);

    VROPlatformDispatchAsyncRenderer([animation_w, node_w, obj_g] {
        if (obj_g == NULL){
            return;
        }

        JNIEnv *env = VROPlatformGetJNIEnv();
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            env->DeleteWeakGlobalRef(obj_g);
            return;
        }
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            env->DeleteWeakGlobalRef(obj_g);
            return;
        }
        animation->execute(node, [obj_g] {
            VROPlatformDispatchAsyncApplication([obj_g] {
                if (obj_g == NULL){
                    return;
                }

                JNIEnv *env = VROPlatformGetJNIEnv();
                jclass javaClass = VROPlatformFindClass(env, obj_g,
                                                        "com/viro/core/internal/ExecutableAnimation");
                if (javaClass == nullptr) {
                    perr("Unable to find ExecutableAnimation class for onFinish callback.");
                    env->DeleteWeakGlobalRef(obj_g);
                    return;
                }

                jmethodID method = env->GetMethodID(javaClass, "animationDidFinish", "()V");
                if (method == nullptr) {
                    perr("Unable to find animationDidFinish() method in ExecutableAnimation");
                    env->DeleteWeakGlobalRef(obj_g);
                    return;
                }

                env->CallVoidMethod(obj_g, method);
                if (env->ExceptionOccurred()) {
                    perr("Exception encountered calling ExecutableAnimation::animationDidFinish");
                }
                env->DeleteLocalRef(javaClass);
                env->DeleteWeakGlobalRef(obj_g);
            });
        });
    });
}

JNI_METHOD(void, nativePauseAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->pause();
    });
}

JNI_METHOD(void, nativeResumeAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->resume();
    });
}

JNI_METHOD(void, nativeTerminateAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    std::weak_ptr<VROExecutableAnimation> animation_w = ExecutableAnimation::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([animation_w] {
        std::shared_ptr<VROExecutableAnimation> animation = animation_w.lock();
        if (!animation) {
            return;
        }
        animation->terminate();
    });
}

JNI_METHOD(void, nativeDestroyAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROExecutableAnimation> *>(nativeRef);
}

} // extern "C"