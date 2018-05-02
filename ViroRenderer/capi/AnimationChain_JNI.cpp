//
//  AnimationChain_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include "VROAnimationChain.h"
#include "VROStringUtil.h"
#include "AnimationGroup_JNI.h"
#include "Node_JNI.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_internal_AnimationChain_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type AnimationChain_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROAnimationChain), nativeCreateAnimationChain)(VRO_ARGS
                                                                   VRO_STRING executionType) {
    VRO_METHOD_PREAMBLE;
    std::vector<std::shared_ptr<VROExecutableAnimation>> emptyChain;
    VROAnimationChainExecution execution = VROAnimationChainExecution::Serial;

    if (VROStringUtil::strcmpinsensitive(VRO_STRING_STL(executionType), "parallel")) {
        execution = VROAnimationChainExecution::Parallel;
    }

    std::shared_ptr<VROAnimationChain> animationChain = std::make_shared<VROAnimationChain>(emptyChain, execution);
    return VRO_REF_NEW(VROAnimationChain, animationChain);
}

VRO_METHOD(VRO_REF(VROAnimationChain), nativeCopyAnimation)(VRO_ARGS
                                                            VRO_REF(VROAnimationChain) nativeRef) {
    std::shared_ptr<VROAnimationChain> chain = VRO_REF_GET(VROAnimationChain, nativeRef);
    return VRO_REF_NEW(VROAnimationChain, std::dynamic_pointer_cast<VROAnimationChain>(chain->copy()));
}

VRO_METHOD(void, nativeAddAnimationChain)(VRO_ARGS
                                          VRO_REF(VROAnimationChain) nativeRef,
                                          VRO_REF(VROAnimationChain) chainRef) {
    VRO_REF_GET(VROAnimationChain, nativeRef)->addAnimation(VRO_REF_GET(VROAnimationChain, chainRef));
}

VRO_METHOD(void, nativeAddAnimationGroup)(VRO_ARGS
                                          VRO_REF(VROAnimationChain) nativeRef,
                                          VRO_REF(AnimationGroup) groupRef) {
    VRO_REF_GET(VROAnimationChain, nativeRef)->addAnimation(VRO_REF_GET(VROAnimationGroup, groupRef));
}

VRO_METHOD(void, nativeDestroyAnimationChain)(VRO_ARGS
                                              VRO_REF(VROAnimationChain) nativeRef) {
    VRO_REF_DELETE(VROAnimationChain, nativeRef);
}

} // extern "C"