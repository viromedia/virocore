//
// Created by Raj Advani on 10/31/17.
//

#ifndef ANDROID_ANIMATIONTRANSACTION_JNI_H
#define ANDROID_ANIMATIONTRANSACTION_JNI_H

#include <memory>
#include <VROTransaction.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace AnimationTransaction {
    inline VRO_REF jptr(std::shared_ptr<VROTransaction> ptr) {
        PersistentRef<VROTransaction> *pref = new PersistentRef<VROTransaction>(ptr);
        return reinterpret_cast<intptr_t>(pref);
    }

    inline std::shared_ptr<VROTransaction> native(VRO_REF ptr) {
        PersistentRef<VROTransaction> *pref = reinterpret_cast<PersistentRef<VROTransaction> *>(ptr);
        return pref->get();
    }
}


#endif //ANDROID_ANIMATIONTRANSACTION_JNI_H
