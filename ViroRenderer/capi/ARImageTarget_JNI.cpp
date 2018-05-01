//
// Created by Andy Chu on 3/8/18.
//

#include <VROStringUtil.h>
#include "ARImageTarget_JNI.h"

#if VRO_PLATFORM_ANDROID
#include "VROARImageTargetAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARImageTarget_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARImageTarget_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARImageTarget)(VRO_ARGS
                                               VRO_OBJECT bitmap,
                                               VRO_STRING orientation,
                                               VRO_FLOAT physicalWidth,
                                               VRO_STRING id) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env);
    std::string strOrientation = VRO_STRING_STL(orientation);

    VROImageOrientation imageOrientation;
    if (VROStringUtil::strcmpinsensitive(strOrientation, "Down")) {
        imageOrientation = VROImageOrientation::Down;
    } else if (VROStringUtil::strcmpinsensitive(strOrientation, "Left")) {
        imageOrientation = VROImageOrientation::Left;

    } else if (VROStringUtil::strcmpinsensitive(strOrientation, "Right")) {
        imageOrientation = VROImageOrientation::Right;
    } else { // "Up"
        imageOrientation = VROImageOrientation::Up;
    }

    std::string strId = VRO_STRING_STL(id);

    std::shared_ptr<VROARImageTarget> target;
#if VRO_PLATFORM_ANDROID
    target = std::make_shared<VROARImageTargetAndroid>(bitmap, imageOrientation, physicalWidth, strId);
#else
    // TODO wasm
#endif

    return ARImageTarget::jptr(target);
}

VRO_METHOD(void, nativeDestroyARImageTarget)(VRO_ARGS
                                             VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROARImageTarget> *>(nativeRef);
}


}