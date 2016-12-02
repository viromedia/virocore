//
//  Box_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>

#include "VROBox.h"
#include "VROMaterial.h"
#include "VROImageAndroid.h"
#include "PersistentRef.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_BoxJni_##method_name

namespace Box {
    inline jlong jptr(std::shared_ptr<VROGeometry> shared_node) {
        PersistentRef<VROGeometry> *native_box = new PersistentRef<VROGeometry>(shared_node);
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
                                        jobject class_loader,
                                        jobject node_jni,
                                        jlong width,
                                        jlong height,
                                        jlong length) {
    std::shared_ptr<VROBox> box = VROBox::createBox(width, height, length);

    /**
     * TODO:
     * Set lighting model and materials to this geometry once we have
     * created it's corresponding managers.
     */
    std::shared_ptr<VROMaterial> _material = box->getMaterials()[0];
    _material->setLightingModel(VROLightingModel::Constant);
    _material->getDiffuse().setTexture(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("boba.png")));
    return Box::jptr(box);
}

JNI_METHOD(void, nativeDestroyBox)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_node_ref) {
    delete reinterpret_cast<PersistentRef<VROBox> *>(native_node_ref);
}

}  // extern "C"
