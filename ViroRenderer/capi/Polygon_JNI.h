//
//  Polygon_JNI.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_POLYGON_JNI_H
#define ANDROID_POLYGON_JNI_H

#include <memory>
#include "PersistentRef.h"
#include "VROPolygon.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Polygon {
    inline VRO_REF(VROPolygon) jptr(std::shared_ptr<VROPolygon> polygon) {
        PersistentRef<VROPolygon> *persistedPolygon = new PersistentRef<VROPolygon>(polygon);
        return reinterpret_cast<intptr_t>(persistedPolygon);
    }

    inline std::shared_ptr<VROPolygon> native(VRO_REF(VROPolygon) ptr) {
        PersistentRef<VROPolygon> *persistedPolygon = reinterpret_cast<PersistentRef<VROPolygon> *>(ptr);
        return persistedPolygon->get();
    }
}

#endif //#define ANDROID_POLYGON_JNI_H
