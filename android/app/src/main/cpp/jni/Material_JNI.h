//
//  Material_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#ifndef Material_JNI_h
#define Material_JNI_h

#include <jni.h>
#include <memory>

#include "VROMaterial.h"
#include "Texture_JNI.h"

namespace Material {
    inline jlong jptr(std::shared_ptr<VROMaterial> ptr) {
        PersistentRef<VROMaterial> *persistentRef = new PersistentRef<VROMaterial>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROMaterial> native(jlong ptr) {
        PersistentRef<VROMaterial> *persistentRef = reinterpret_cast<PersistentRef<VROMaterial> *>(ptr);
        return persistentRef->get();
    }
}
#endif