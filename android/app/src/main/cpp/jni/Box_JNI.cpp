//
//  Box_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>
#include "VROBox.h"
#include "VROMaterial.h"
#include "PersistentRef.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Box_##method_name
#endif

namespace Box {
    inline VRO_REF jptr(std::shared_ptr<VROBox> shared_node) {
        PersistentRef<VROBox> *native_box = new PersistentRef<VROBox>(shared_node);
        return reinterpret_cast<intptr_t>(native_box);
    }

    inline std::shared_ptr<VROBox> native(VRO_REF ptr) {
        PersistentRef<VROBox> *persistentBox = reinterpret_cast<PersistentRef<VROBox> *>(ptr);
        return persistentBox->get();
    }
}

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateBox)(VRO_ARGS
                                     jfloat width,
                                     jfloat height,
                                     jfloat length) {
    std::shared_ptr<VROBox> box = VROBox::createBox(width, height, length);
    return Box::jptr(box);
}

VRO_METHOD(void, nativeDestroyBox)(JNIEnv *env,
                                        jclass clazz,
                                        VRO_REF nativeBoxRef) {
    delete reinterpret_cast<PersistentRef<VROBox> *>(nativeBoxRef);
}

VRO_METHOD(void, nativeSetWidth)(VRO_ARGS
                                 VRO_REF native_box_ref,
                                 jfloat width) {
    std::weak_ptr<VROBox> box_w = Box::native(native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, width] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setWidth(width);
        }
    });
}

VRO_METHOD(void, nativeSetHeight)(VRO_ARGS
                                  VRO_REF native_box_ref,
                                  jfloat height) {
    std::weak_ptr<VROBox> box_w = Box::native(native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, height] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setHeight(height);
        }
    });
}

VRO_METHOD(void, nativeSetLength)(VRO_ARGS
                                  VRO_REF native_box_ref,
                                  jfloat length) {
    std::weak_ptr<VROBox> box_w = Box::native(native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, length] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setLength(length);
        }
    });
}

}  // extern "C"
