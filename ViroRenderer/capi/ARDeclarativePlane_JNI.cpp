//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROARDeclarativePlane.h>
#include <VROStringUtil.h>
#include "ARDeclarativePlane_JNI.h"
#include "ARUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ARDeclarativePlane_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARDeclarativePlane_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARPlane)(VRO_ARGS
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
    return ARDeclarativePlane::jptr(arPlane);
}

VRO_METHOD(void, nativeSetMinWidth)(VRO_ARGS
                                    VRO_REF nativeARPlane,
                                    VRO_FLOAT minWidth) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

VRO_METHOD(void, nativeSetMinHeight)(VRO_ARGS
                                     VRO_REF nativeARPlane,
                                     VRO_FLOAT minHeight) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

VRO_METHOD(void, nativeSetAlignment)(VRO_ARGS
                                     VRO_REF nativeARPlane,
                                     VRO_STRING jAlignment) {
    std::string strAlignment = VRO_STRING_STL(jAlignment);
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
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


