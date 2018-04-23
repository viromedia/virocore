//
//  AnimationGroup_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "AnimationGroup_JNI.h"
#include "Node_JNI.h"
#include "LazyMaterial_JNI.h"
#include <VROPlatformUtil.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_internal_AnimationGroup_##method_name
#endif

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

VRO_METHOD(VRO_REF, nativeCreateAnimationGroup)(VRO_ARGS
                                                jstring positionX, jstring positionY, jstring positionZ,
                                                jstring scaleX, jstring scaleY, jstring scaleZ,
                                                jstring rotateX, jstring rotateY, jstring rotateZ,
                                                jstring opacity, jstring color, VRO_REF lazyMaterialRef,
                                                jfloat durationSeconds, jfloat delaySeconds, jstring functionType) {
    std::map<std::string, std::string> animationProperties;

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

    std::vector<std::shared_ptr<VROLazyMaterial>> materialAnimations;
    if (lazyMaterialRef != 0) {
        std::shared_ptr<VROLazyMaterial> lazyMaterial = LazyMaterial::native(lazyMaterialRef);
        materialAnimations.push_back(lazyMaterial);
    }

    std::string functionTypeStr = VROPlatformGetString(functionType, env);
    std::shared_ptr<VROAnimationGroup> animationGroup = VROAnimationGroup::parse(durationSeconds, delaySeconds,
                                                                                 functionTypeStr, animationProperties,
                                                                                 materialAnimations);
    return AnimationGroup::jptr(animationGroup);
}

VRO_METHOD(VRO_REF, nativeCopyAnimation)(VRO_ARGS
                                         VRO_REF nativeRef) {
    std::shared_ptr<VROAnimationGroup> group = AnimationGroup::native(nativeRef);
    return AnimationGroup::jptr(std::dynamic_pointer_cast<VROAnimationGroup>(group->copy()));
}

VRO_METHOD(void, nativeDestroyAnimationGroup)(VRO_ARGS
                                              VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROAnimationGroup> *>(nativeRef);
}

} // extern "C"