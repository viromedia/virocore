#ifndef ANDROID_ARIMAGETARGET_JNI_H
#define ANDROID_ARIMAGETARGET_JNI_H

#include <memory>
#include "VROARImageTarget.h"
#include "VROPlatformUtil.h"
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARImageTarget {
    inline VRO_REF jptr(std::shared_ptr<VROARImageTarget> target) {
        PersistentRef<VROARImageTarget> *target_p = new PersistentRef<VROARImageTarget>(target);
        return reinterpret_cast<intptr_t>(target_p);
    }

    inline std::shared_ptr<VROARImageTarget> native(VRO_REF target_j) {
        PersistentRef<VROARImageTarget> *target_p = reinterpret_cast<PersistentRef<VROARImageTarget> *>(target_j);
        return target_p->get();
    }
};


#endif //ANDROID_ARIMAGETARGET_JNI_H
