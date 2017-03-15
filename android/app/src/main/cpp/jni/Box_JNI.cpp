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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_BoxJni_##method_name

namespace Box {
    inline jlong jptr(std::shared_ptr<VROBox> shared_node) {
        PersistentRef<VROBox> *native_box = new PersistentRef<VROBox>(shared_node);
        return reinterpret_cast<intptr_t>(native_box);
    }

    inline std::shared_ptr<VROBox> native(jlong ptr) {
        PersistentRef<VROBox> *persistentBox = reinterpret_cast<PersistentRef<VROBox> *>(ptr);
        return persistentBox->get();
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreateBox)(JNIEnv *env,
                                        jclass clazz,
                                        jfloat width,
                                        jfloat height,
                                        jfloat length) {
    std::shared_ptr<VROBox> box = VROBox::createBox(width, height, length);
    return Box::jptr(box);
}

JNI_METHOD(void, nativeDestroyBox)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativeBoxRef) {
    delete reinterpret_cast<PersistentRef<VROBox> *>(nativeBoxRef);
}

JNI_METHOD(void, nativeAttachToNode)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_box_ref,
                                     jlong native_node_ref) {
    std::shared_ptr<VROBox> box = Box::native(native_box_ref);
    std::shared_ptr<VRONode> node = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([box, node] {
        node->setGeometry(box);
    });
}

}  // extern "C"
