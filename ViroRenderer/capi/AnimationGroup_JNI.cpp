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
 * Note: this function releases the VRO_STRING candidate before returning!
 */
void AddPropertyIfNotNull(VRO_ENV env, std::string property, VRO_STRING candidate,
                          std::map<std::string, std::string> &propertyMap) {
    const char *candidateCStr = nullptr;
    if (!VRO_IS_STRING_EMPTY(candidate)) {
        std::string candidateStr = VROPlatformGetString(candidate, env);
        propertyMap[property] = candidateStr;
    }
}

VRO_METHOD(VRO_REF, nativeCreateAnimationGroup)(VRO_ARGS
                                                VRO_STRING positionX, VRO_STRING positionY, VRO_STRING positionZ,
                                                VRO_STRING scaleX, VRO_STRING scaleY, VRO_STRING scaleZ,
                                                VRO_STRING rotateX, VRO_STRING rotateY, VRO_STRING rotateZ,
                                                VRO_STRING opacity, VRO_STRING color, VRO_REF lazyMaterialRef,
                                                VRO_FLOAT durationSeconds, VRO_FLOAT delaySeconds, VRO_STRING functionType) {
    std::map<std::string, std::string> animationProperties;

    // NOTE: AddPropertyIfNotNull WILL release the VRO_STRING after its done running so don't
    // use the VRO_STRING after!
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