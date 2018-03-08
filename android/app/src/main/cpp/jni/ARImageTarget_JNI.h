#ifndef ANDROID_ARIMAGETARGET_JNI_H
#define ANDROID_ARIMAGETARGET_JNI_H

#include <jni.h>
#include <memory>
#include <VROARImageTargetAndroid.h>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"

namespace ARImageTarget {
    inline jlong jptr(std::shared_ptr<VROARImageTargetAndroid> target) {
        PersistentRef<VROARImageTargetAndroid> *target_p = new PersistentRef<VROARImageTargetAndroid>(target);
        return reinterpret_cast<intptr_t>(target_p);
    }

    inline std::shared_ptr<VROARImageTargetAndroid> native(jlong target_j) {
        PersistentRef<VROARImageTargetAndroid> *target_p = reinterpret_cast<PersistentRef<VROARImageTargetAndroid> *>(target_j);
        return target_p->get();
    }
};


#endif //ANDROID_ARIMAGETARGET_JNI_H
