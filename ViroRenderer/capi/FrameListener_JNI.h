/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#ifndef ANDROID_FRAMELISTENER_JNI_H
#define ANDROID_FRAMELISTENER_JNI_H

#include <memory>
#include "VROFrameListener.h"
#include "VROPlatformUtil.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class FrameListenerJNI : public VROFrameListener {
public:
    FrameListenerJNI(VRO_OBJECT obj) :
        _javaObject(VRO_OBJECT_NULL) {

        VRO_ENV env = VROPlatformGetJNIEnv();
        _javaObject = VRO_NEW_WEAK_GLOBAL_REF(obj);
    }

    virtual ~FrameListenerJNI() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    virtual void onFrameWillRender(const VRORenderContext &context) {
        // do nothing
    }

    virtual void onFrameDidRender(const VRORenderContext &context) {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
        VROPlatformDispatchAsyncApplication([jObjWeak] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
            if (VRO_IS_OBJECT_NULL(localObj)) {
                return;
            }

            VROPlatformCallHostFunction(localObj, "onFrameDidRender", "()V");

            VRO_DELETE_LOCAL_REF(localObj);
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
        });
    }

private:
    VRO_OBJECT _javaObject;
};


#endif //ANDROID_FRAMELISTENER_JNI_H
