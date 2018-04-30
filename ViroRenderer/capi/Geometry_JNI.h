//
// Created by Raj Advani on 10/11/17.
//

#ifndef ANDROID_GEOMETRY_JNI_H
#define ANDROID_GEOMETRY_JNI_H

#include <memory>
#include "VRONode.h"
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Geometry {
    inline VRO_REF jptr(std::shared_ptr<VROGeometry> shared_geo) {
        PersistentRef<VROGeometry> *native_geo = new PersistentRef<VROGeometry>(shared_geo);
        return reinterpret_cast<intptr_t>(native_geo);
    }

    inline std::shared_ptr<VROGeometry> native(VRO_REF ptr) {
        PersistentRef<VROGeometry> *persistentGeo = reinterpret_cast<PersistentRef<VROGeometry> *>(ptr);
        return persistentGeo->get();
    }
}



#endif //ANDROID_GEOMETRY_JNI_H
