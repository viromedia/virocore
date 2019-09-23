//
//  AnimationGroup_JNI.cpp
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

#include "AnimationGroup_JNI.h"
#include "Node_JNI.h"
#include "LazyMaterial_JNI.h"
#include <VROPlatformUtil.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_internal_AnimationGroup_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type AnimationGroup_##method_name
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
        std::string candidateStr = VRO_STRING_STL(candidate);
        propertyMap[property] = candidateStr;
    }
}

VRO_METHOD(VRO_REF(VROAnimationGroup), nativeCreateAnimationGroup)(VRO_ARGS
                                                                   VRO_STRING positionX, VRO_STRING positionY, VRO_STRING positionZ,
                                                                   VRO_STRING scaleX, VRO_STRING scaleY, VRO_STRING scaleZ,
                                                                   VRO_STRING rotateX, VRO_STRING rotateY, VRO_STRING rotateZ,
                                                                   VRO_STRING opacity, VRO_STRING color, VRO_REF(VROLazyMaterial) lazyMaterialRef,
                                                                   VRO_FLOAT durationSeconds, VRO_FLOAT delaySeconds, VRO_STRING functionType) {
    VRO_METHOD_PREAMBLE;
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
        std::shared_ptr<VROLazyMaterial> lazyMaterial = VRO_REF_GET(VROLazyMaterial, lazyMaterialRef);
        materialAnimations.push_back(lazyMaterial);
    }

    std::string functionTypeStr = VRO_STRING_STL(functionType);
    std::shared_ptr<VROAnimationGroup> animationGroup = VROAnimationGroup::parse(durationSeconds, delaySeconds,
                                                                                 functionTypeStr, animationProperties,
                                                                                 materialAnimations);
    return VRO_REF_NEW(VROAnimationGroup, animationGroup);
}

VRO_METHOD(VRO_REF(VROAnimationGroup), nativeCopyAnimation)(VRO_ARGS
                                                            VRO_REF(VROAnimationGroup) nativeRef) {
    std::shared_ptr<VROAnimationGroup> group = VRO_REF_GET(VROAnimationGroup, nativeRef);
    return VRO_REF_NEW(VROAnimationGroup, std::dynamic_pointer_cast<VROAnimationGroup>(group->copy()));
}

VRO_METHOD(void, nativeDestroyAnimationGroup)(VRO_ARGS
                                              VRO_REF(VROAnimationGroup) nativeRef) {
    VRO_REF_DELETE(VROAnimationGroup, nativeRef);
}

} // extern "C"