/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include <memory>
#include <VROPortalFrame.h>
#include "Portal_JNI.h"
#include "PersistentRef.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Portal_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreatePortal)(VRO_NO_ARGS) {
    std::shared_ptr<VROPortalFrame> portal = std::make_shared<VROPortalFrame>();
    return Portal::jptr(portal);
}

VRO_METHOD(void, nativeDestroyPortal)(VRO_ARGS
                                      VRO_REF native_ref) {
    delete reinterpret_cast<PersistentRef<VROPortalFrame> *>(native_ref);
}

}