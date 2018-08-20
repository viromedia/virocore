//
// Created by Andy Chu on 8/20/18.
//

#include "ARImageDatabaseLoaderDelegate.h"
#include "VROPlatformUtil.h"

ARImageDatabaseLoaderDelegate::ARImageDatabaseLoaderDelegate(VRO_OBJECT javaObject, VRO_ENV env) {
    _javaObject = VRO_NEW_WEAK_GLOBAL_REF(javaObject);
}

ARImageDatabaseLoaderDelegate::~ARImageDatabaseLoaderDelegate() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
};

void ARImageDatabaseLoaderDelegate::loadSuccess() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
            return;
        }

        // invoke the function
        VROPlatformCallHostFunction(localObj, "onLoadARImageDatabaseSuccess", "()V");

        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}
void ARImageDatabaseLoaderDelegate::loadFailure(std::string error) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
            return;
        }

        VRO_STRING jerror = VRO_NEW_STRING(error.c_str());

        // invoke the function
        VROPlatformCallHostFunction(localObj, "onLoadARImageDatabaseError", "(Ljava/lang/String;)V", jerror);

        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_LOCAL_REF(jerror);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}