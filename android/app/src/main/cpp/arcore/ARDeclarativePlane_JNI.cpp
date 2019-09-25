//
//  ARPlane_JNI.cpp
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
#include <VROARDeclarativePlane.h>
#include <VROStringUtil.h>
#include "ARDeclarativePlane_JNI.h"
#include "ViroUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ARDeclarativePlane_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARDeclarativePlane_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARDeclarativePlane), nativeCreateARPlane)(VRO_ARGS
                                                                VRO_FLOAT minWidth,
                                                                VRO_FLOAT minHeight,
                                                                VRO_STRING jAlignment) {

    std::string strAlignment = VRO_STRING_STL(jAlignment);
    VROARPlaneAlignment alignment = VROARPlaneAlignment::Horizontal;

    if (VROStringUtil::strcmpinsensitive(strAlignment, "Horizontal")) {
        alignment = VROARPlaneAlignment::Horizontal;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalUpward")) {
        alignment = VROARPlaneAlignment::HorizontalUpward;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalDownward")) {
        alignment = VROARPlaneAlignment::HorizontalDownward;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "Vertical")) {
        alignment = VROARPlaneAlignment::Vertical;
    }

    std::shared_ptr<VROARDeclarativePlane> arPlane = std::make_shared<VROARDeclarativePlane>(
            minWidth, minHeight, alignment);
    return VRO_REF_NEW(VROARDeclarativePlane, arPlane);
}

VRO_METHOD(void, nativeSetMinWidth)(VRO_ARGS
                                    VRO_REF(VROARDeclarativePlane) nativeARPlane,
                                    VRO_FLOAT minWidth) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = VRO_REF_GET(VROARDeclarativePlane, nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

VRO_METHOD(void, nativeSetMinHeight)(VRO_ARGS
                                     VRO_REF(VROARDeclarativePlane) nativeARPlane,
                                     VRO_FLOAT minHeight) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = VRO_REF_GET(VROARDeclarativePlane, nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

VRO_METHOD(void, nativeSetAlignment)(VRO_ARGS
                                     VRO_REF(VROARDeclarativePlane) nativeARPlane,
                                     VRO_STRING jAlignment) {
    std::string strAlignment = VRO_STRING_STL(jAlignment);
    std::shared_ptr<VROARDeclarativePlane> arPlane = VRO_REF_GET(VROARDeclarativePlane, nativeARPlane);
    if (VROStringUtil::strcmpinsensitive(strAlignment, "Horizontal")) {
        arPlane->setAlignment(VROARPlaneAlignment::Horizontal);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalUpward")) {
        arPlane->setAlignment(VROARPlaneAlignment::HorizontalUpward);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalDownward")) {
        arPlane->setAlignment(VROARPlaneAlignment::HorizontalDownward);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "Vertical")) {
        arPlane->setAlignment(VROARPlaneAlignment::Vertical);
    }
}

} // extern "C"


