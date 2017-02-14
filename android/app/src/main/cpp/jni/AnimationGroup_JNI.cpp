//
//  AnimationGroup_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "AnimationGroup_JNI.h"
#include "Node_JNI.h"
#include <VROPlatformUtil.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_AnimationGroupJni_##method_name

extern "C" {

/*
 * Helper function that adds the candidate to the propertyMap if it isnt null.
 *
 * Note: this function releases the jstring candidate before returning!
 */
void AddPropertyIfNotNull(JNIEnv *env, std::string property, jstring candidate,
                          std::map<std::string, std::string> &propertyMap) {
    const char *candidateCStr = nullptr;
    if (candidate != NULL) {
        candidateCStr = env->GetStringUTFChars(candidate, NULL);
        std::string candidateStr(candidateCStr);
        propertyMap[property] = candidateStr;
        env->ReleaseStringUTFChars(candidate, candidateCStr);
    }
}

JNI_METHOD(jlong, nativeCreateAnimationGroup)(JNIEnv *env, jclass clazz,
                                              jstring positionX, jstring positionY, jstring positionZ,
                                              jstring scaleX, jstring scaleY, jstring scaleZ,
                                              jstring rotateX, jstring rotateY, jstring rotateZ,
                                              jstring opacity, jstring color, jfloat durationSeconds,
                                              jfloat delaySeconds, jstring functionType) {
    std::map<std::string, std::string> animationProperties;
    std::vector<std::shared_ptr<VROLazyMaterial>> materialAnimations;
    // NOTE: AddPropertyIfNotNull WILL release the jstring after its done running so don't
    // use the jstring after!
    AddPropertyIfNotNull(env, "positionX", positionX, animationProperties);
    AddPropertyIfNotNull(env, "positionY", positionY, animationProperties);
    AddPropertyIfNotNull(env, "positionZ", positionZ, animationProperties);
    AddPropertyIfNotNull(env, "scaleX", scaleX, animationProperties);
    AddPropertyIfNotNull(env, "scaleY", scaleY, animationProperties);
    AddPropertyIfNotNull(env, "scaleZ", scaleZ, animationProperties);
    AddPropertyIfNotNull(env, "rotateX", rotateX, animationProperties);
    AddPropertyIfNotNull(env, "rotateY", rotateY, animationProperties);
    AddPropertyIfNotNull(env, "rotateZ", rotateZ, animationProperties);
    AddPropertyIfNotNull(env, "opacity", opacity, animationProperties);
    AddPropertyIfNotNull(env, "color", color, animationProperties);

    const char *functionTypeCStr = env->GetStringUTFChars(functionType, NULL);
    std::string functionTypeStr(functionTypeCStr);

    std::shared_ptr<VROAnimationGroup> animationGroup = VROAnimationGroup::parse(durationSeconds, delaySeconds,
                                                                                 functionTypeStr, animationProperties,
                                                                                 materialAnimations);
    env->ReleaseStringUTFChars(functionType, functionTypeCStr);
    return AnimationGroup::jptr(animationGroup);
}

JNI_METHOD(void, nativeExecuteAnimation)(JNIEnv *env, jobject obj, jlong nativeRef, jlong nodeRef) {
    jweak weakObj = env->NewWeakGlobalRef(obj);

    VROPlatformDispatchAsyncRenderer([nativeRef, nodeRef, weakObj] {
        AnimationGroup::native(nativeRef)->execute(Node::native(nodeRef), [weakObj] {
            JNIEnv *env = VROPlatformGetJNIEnv();
            jobject obj = env->NewLocalRef(weakObj);

            if (obj != NULL) {
                jclass javaClass = VROPlatformFindClass(env, obj,
                                                        "com/viro/renderer/jni/AnimationGroupJni");
                if (javaClass == nullptr) {
                    perr("Unable to find AnimationGroupJni class for onFinish callback.");
                    return;
                }

                jmethodID method = env->GetMethodID(javaClass, "animationDidFinish", "()V");
                if (method == nullptr) {
                    perr("Unable to find animationDidFinish() method in AnimationGroupJni");
                }

                env->CallVoidMethod(obj, method);
                if (env->ExceptionOccurred()) {
                    perr("Exception encountered calling onFinish.");
                }
                env->DeleteLocalRef(javaClass);
            }
        });
    });
}

JNI_METHOD(void, nativePauseAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    AnimationGroup::native(nativeRef)->pause();
}

JNI_METHOD(void, nativeResumeAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    AnimationGroup::native(nativeRef)->resume();
}

JNI_METHOD(void, nativeTerminateAnimation)(JNIEnv *env, jobject obj, jlong nativeRef) {
    AnimationGroup::native(nativeRef)->terminate();
}

JNI_METHOD(void, nativeDestroyAnimationGroup)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROAnimationGroup> *>(nativeRef);
}

} // extern "C"