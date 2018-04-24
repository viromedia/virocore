//
// Created by Andy Chu on 3/8/18.
//

#include <VROStringUtil.h>
#include "ARImageTarget_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARImageTarget_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARImageTarget)(VRO_ARGS
                                               jobject bitmap,
                                               VRO_STRING orientation,
                                               VRO_FLOAT physicalWidth,
                                               VRO_STRING id) {
    VROPlatformSetEnv(env);

    std::string strOrientation = VROPlatformGetString(orientation, env);

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

    std::string strId = VROPlatformGetString(id, env);

    std::shared_ptr<VROARImageTargetAndroid> target =
            std::make_shared<VROARImageTargetAndroid>(bitmap, imageOrientation, physicalWidth, strId);

    return ARImageTarget::jptr(target);
}

VRO_METHOD(void, nativeDestroyARImageTarget)(VRO_ARGS
                                             VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROARImageTargetAndroid> *>(nativeRef);
}


}