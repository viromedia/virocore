//
//  Camera_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_CAMERA_JNI_H
#define ANDROID_CAMERA_JNI_H

#include <jni.h>
#include <memory>
#include <VRONodeCamera.h>
#include "PersistentRef.h"

namespace Camera {
    inline jlong jptr(std::shared_ptr<VRONodeCamera> shared_camera) {
        PersistentRef<VRONodeCamera> *native_camera = new PersistentRef<VRONodeCamera>(shared_camera);
        return reinterpret_cast<intptr_t>(native_camera);
    }

    inline std::shared_ptr<VRONodeCamera> native(jlong ptr) {
        PersistentRef<VRONodeCamera> *persistentCamera = reinterpret_cast<PersistentRef<VRONodeCamera> *>(ptr);
        return persistentCamera->get();
    }
}


#endif //ANDROID_CAMERA_JNI_H
