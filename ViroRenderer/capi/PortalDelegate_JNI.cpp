/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include <memory>
#include <VROPlatformUtil.h>
#include "PortalDelegate_JNI.h"

PortalDelegate::PortalDelegate(VRO_OBJECT javaPortalSceneObject) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    _javaObject = reinterpret_cast<jclass>(VRO_NEW_WEAK_GLOBAL_REF(javaPortalSceneObject));
}

PortalDelegate::~PortalDelegate() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}

void PortalDelegate::onPortalEnter() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onPortalEnter", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}



void PortalDelegate::onPortalExit() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onPortalExit", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

