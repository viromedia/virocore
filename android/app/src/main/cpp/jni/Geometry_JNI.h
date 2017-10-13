//
// Created by Raj Advani on 10/11/17.
//

#ifndef ANDROID_GEOMETRY_JNI_H
#define ANDROID_GEOMETRY_JNI_H

#include <jni.h>
#include <memory>
#include <VRONode.h>
#include "PersistentRef.h"

namespace Geometry {
    inline jlong jptr(std::shared_ptr<VROGeometry> shared_geo) {
        PersistentRef<VROGeometry> *native_geo = new PersistentRef<VROGeometry>(shared_geo);
        return reinterpret_cast<intptr_t>(native_geo);
    }

    inline std::shared_ptr<VROGeometry> native(jlong ptr) {
        PersistentRef<VROGeometry> *persistentGeo = reinterpret_cast<PersistentRef<VROGeometry> *>(ptr);
        return persistentGeo->get();
    }
}



#endif //ANDROID_GEOMETRY_JNI_H
