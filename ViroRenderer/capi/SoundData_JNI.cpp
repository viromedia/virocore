//
//  SoundData_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include <VROPlatformUtil.h>
#include "SoundData_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_SoundData_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type SoundData_##method_name
#endif

extern "C" {

    VRO_METHOD(VRO_REF(VROSoundDataGVR), nativeCreateSoundData)(VRO_ARGS
                                                                VRO_STRING filepath) {
        VRO_METHOD_PREAMBLE;
        std::string path = VRO_STRING_STL(filepath);

        // Set the platform env because the renderer could've not been initialized yet (and set
        // the env)
        VROPlatformSetEnv(env);
        std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(path, VROResourceType::URL);
        return VRO_REF_NEW(VROSoundDataGVR, data);
    }

    VRO_METHOD(VRO_REF(VROSoundDataDelegate_JNI), nativeSetSoundDataDelegate)(VRO_ARGS
                                                                              VRO_REF(VROSoundDataGVR) nativeRef) {
        VRO_METHOD_PREAMBLE;

        std::shared_ptr<VROSoundDataGVR> data = VRO_REF_GET(VROSoundDataGVR, nativeRef);
        std::shared_ptr<VROSoundDataDelegate_JNI> delegateRef
                = std::make_shared<VROSoundDataDelegate_JNI>(obj, env);
        data->setDelegate(delegateRef);
        return VRO_REF_NEW(VROSoundDataDelegate_JNI, delegateRef);
    }

    VRO_METHOD(void, nativeDestroySoundData)(VRO_ARGS
                                             VRO_REF(VROSoundDataGVR) nativeRef) {
        VRO_REF_DELETE(VROSoundDataGVR, nativeRef);
    }

    VRO_METHOD(void, nativeDestroySoundDataDelegate)(VRO_ARGS
                                                     VRO_REF(VROSoundDataDelegate_JNI) nativeRef) {
        VRO_REF_DELETE(VROSoundDataDelegate_JNI, nativeRef);
    }
} // extern "C"

VROSoundDataDelegate_JNI::VROSoundDataDelegate_JNI(VRO_OBJECT nodeJavaObject, VRO_ENV env) :
    _javaObject(VRO_NEW_WEAK_GLOBAL_REF(nodeJavaObject)) {
}

VROSoundDataDelegate_JNI::~VROSoundDataDelegate_JNI() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void VROSoundDataDelegate_JNI::dataIsReady(){
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
            return;
        }

        VROPlatformCallHostFunction(localObj, "dataIsReady", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VROSoundDataDelegate_JNI::dataError(std::string error){
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
        VROPlatformCallHostFunction(localObj, "dataError", "(Ljava/lang/String;)V", jerror);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
        VRO_DELETE_LOCAL_REF(jerror);
    });
}