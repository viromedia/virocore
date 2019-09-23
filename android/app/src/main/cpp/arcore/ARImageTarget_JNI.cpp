//
//  Created by Andy Chu on 3/8/18.
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

#include <VROStringUtil.h>
#include "ARImageTarget_JNI.h"

#if VRO_PLATFORM_ANDROID
#include "VROARImageTargetAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARImageTarget_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARImageTarget_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARImageTarget), nativeCreateARImageTarget)(VRO_ARGS
                                                                 VRO_OBJECT bitmap,
                                                                 VRO_STRING orientation,
                                                                 VRO_FLOAT physicalWidth,
                                                                 VRO_STRING id) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env);
    std::string strOrientation = VRO_STRING_STL(orientation);

    VROImageOrientation imageOrientation;
    if (VROStringUtil::strcmpinsensitive(strOrientation, "Down")) {
        imageOrientation = VROImageOrientation::Down;
    } else if (VROStringUtil::strcmpinsensitive(strOrientation, "Left")) {
        imageOrientation = VROImageOrientation::Left;

    } else if (VROStringUtil::strcmpinsensitive(strOrientation, "Right")) {
        imageOrientation = VROImageOrientation::Right;
    } else { // "Up"
        imageOrientation = VROImageOrientation::Up;
    }

    std::string strId = VRO_STRING_STL(id);

    std::shared_ptr<VROARImageTarget> target;
#if VRO_PLATFORM_ANDROID
    target = std::make_shared<VROARImageTargetAndroid>(bitmap, imageOrientation, physicalWidth, strId);
#else
    // TODO wasm
#endif

    return VRO_REF_NEW(VROARImageTarget, target);
}

VRO_METHOD(void, nativeDestroyARImageTarget)(VRO_ARGS
                                             VRO_REF(VROARImageTarget) nativeRef) {
    VRO_REF_DELETE(VROARImageTarget, nativeRef);
}


}