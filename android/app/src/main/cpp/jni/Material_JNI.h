//
//  Material_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

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

    inline bool strcmpinsensitive(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (int i = 0; i < a.size(); i++) {
            if (tolower(a[i]) != tolower(b[i])) {
                return false;
            }
        }
        return true;
    }
}