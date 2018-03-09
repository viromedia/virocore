//
// Created by Andy Chu on 3/8/18.
//

#include <VROStringUtil.h>
#include "ARImageTarget_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARImageTarget_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARImageTarget) (JNIEnv *env,
                                             jclass clazz,
                                             jobject bitmap,
                                             jstring orientation,
                                             jfloat physicalWidth,
                                             jstring id) {

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

JNI_METHOD(void, nativeDestroyARImageTarget)(JNIEnv *env, jclass clazz, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROARImageTargetAndroid> *>(nativeRef);
}


}