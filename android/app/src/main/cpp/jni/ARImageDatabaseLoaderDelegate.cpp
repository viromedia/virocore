//
//  Created by Andy Chu on 8/20/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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