#include <jni.h>
#include <memory>

#include "VROImageAndroid.h"
#include "VROTexture.h"
#include "PersistentRef.h"

namespace Image {
    inline jlong jptr(std::shared_ptr<VROImageAndroid> ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = new PersistentRef<VROImageAndroid>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROImageAndroid> native(jlong ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = reinterpret_cast<PersistentRef<VROImageAndroid> *>(ptr);
        return persistentRef->get();
    }
}

namespace Texture {
    inline jlong jptr(std::shared_ptr<VROTexture> ptr) {
        PersistentRef<VROTexture> *persistentRef = new PersistentRef<VROTexture>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROTexture> native(jlong ptr) {
        PersistentRef<VROTexture> *persistentRef = reinterpret_cast<PersistentRef<VROTexture> *>(ptr);
        return persistentRef->get();
    }

}