//
//  FrameListener_JNI.h
//  ViroRenderer
//
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
