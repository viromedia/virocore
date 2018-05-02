//
//  AnimationChain_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include "VROAnimationChain.h"
#include "PersistentRef.h"
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

namespace AnimationChain {
    inline VRO_REF(VROAnimationChain) jptr(std::shared_ptr<VROAnimationChain> ptr) {
        PersistentRef<VROAnimationChain> *persistentRef = new PersistentRef<VROAnimationChain>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROAnimationChain> native(VRO_REF(VROAnimationChain) ptr) {
        PersistentRef<VROAnimationChain> *persistentRef = reinterpret_cast<PersistentRef<VROAnimationChain> *>(ptr);
        return persistentRef->get();
    }
}

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
    return AnimationChain::jptr(animationChain);
}

VRO_METHOD(VRO_REF(VROAnimationChain), nativeCopyAnimation)(VRO_ARGS
                                                            VRO_REF(VROAnimationChain) nativeRef) {
    std::shared_ptr<VROAnimationChain> chain = AnimationChain::native(nativeRef);
    return AnimationChain::jptr(std::dynamic_pointer_cast<VROAnimationChain>(chain->copy()));
}

VRO_METHOD(void, nativeAddAnimationChain)(VRO_ARGS
                                          VRO_REF(VROAnimationChain) nativeRef,
                                          VRO_REF(VROAnimationChain) chainRef) {
    AnimationChain::native(nativeRef)->addAnimation(AnimationChain::native(chainRef));
}

VRO_METHOD(void, nativeAddAnimationGroup)(VRO_ARGS
                                          VRO_REF(VROAnimationChain) nativeRef,
                                          VRO_REF(AnimationGroup) groupRef) {
    AnimationChain::native(nativeRef)->addAnimation(AnimationGroup::native(groupRef));
}

VRO_METHOD(void, nativeDestroyAnimationChain)(VRO_ARGS
                                              VRO_REF(VROAnimationChain) nativeRef) {
    delete reinterpret_cast<PersistentRef<VROAnimationChain> *>(nativeRef);
}

} // extern "C"