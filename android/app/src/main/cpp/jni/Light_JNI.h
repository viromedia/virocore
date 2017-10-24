//
// Created by Raj Advani on 10/24/17.
//

#ifndef ANDROID_LIGHT_JNI_H
#define ANDROID_LIGHT_JNI_H

#include <jni.h>
#include <VROLight.h>
#include <PersistentRef.h>

namespace Light {
    inline jlong jptr(std::shared_ptr<VROLight> shared_node) {
        PersistentRef<VROLight> *native_light = new PersistentRef<VROLight>(shared_node);
        return reinterpret_cast<intptr_t>(native_light);
    }

    inline std::shared_ptr<VROLight> native(jlong ptr) {
        PersistentRef<VROLight> *persistentBox = reinterpret_cast<PersistentRef<VROLight> *>(ptr);
        return persistentBox->get();
    }
}

#endif //ANDROID_LIGHT_JNI_H
