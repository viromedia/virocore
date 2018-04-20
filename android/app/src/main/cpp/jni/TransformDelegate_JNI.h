//
//  TransformDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef TransformDelegate_JNI_H
#define TransformDelegate_JNI_H

#include <PersistentRef.h>
#include <VROTransformDelegate.h>
#include "VROVideoDelegateInternal.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class TransformDelegate_JNI : public VROTransformDelegate {
public:
    TransformDelegate_JNI(jobject delegateJavaObject, double distanceFilter);
    ~TransformDelegate_JNI();

    static VRO_REF jptr(std::shared_ptr<TransformDelegate_JNI> shared_node) {
        PersistentRef<TransformDelegate_JNI> *native_surface = new PersistentRef<TransformDelegate_JNI>(shared_node);
        return reinterpret_cast<intptr_t>(native_surface);
    }

    static std::shared_ptr<TransformDelegate_JNI> native(VRO_REF ptr) {
        PersistentRef<TransformDelegate_JNI> *persistentSurface = reinterpret_cast<PersistentRef<TransformDelegate_JNI> *>(ptr);
        return persistentSurface->get();
    }

    /*
     Notification delegate to let the bridge know that the position has changed.
     */
    void onPositionUpdate(VROVector3f position);

private:
    jobject _javaObject;
};
#endif
