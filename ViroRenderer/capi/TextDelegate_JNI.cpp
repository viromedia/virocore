//
//  TextDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include "TextDelegate_JNI.h"
#include <VROPlatformUtil.h>

TextDelegate::TextDelegate(VRO_OBJECT textJavaObject, VRO_ENV env) :
    _javaObject(VRO_NEW_WEAK_GLOBAL_REF(textJavaObject)) {
}

TextDelegate::~TextDelegate() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void TextDelegate::textCreated(VRO_REF nativeTextRef) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, nativeTextRef] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "textDidFinishCreation", "(J)V", nativeTextRef);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}
