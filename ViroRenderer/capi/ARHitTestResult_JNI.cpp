//
// Created by Raj Advani on 5/19/18.
//

#include <memory>
#include <arcore/VROARHitTestResultARCore.h>
#include <arcore/VROARAnchorARCore.h>
#include "VROARImageTarget.h"
#include "VROPlatformUtil.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARHitTestResult_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARHitTestResult_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARNode), nativeCreateAnchoredNode)(VRO_ARGS
                                                         VRO_REF(VROARHitTestResultARCore) hit_j) {

    std::shared_ptr<VROARHitTestResultARCore> hit = VRO_REF_GET(VROARHitTestResultARCore, hit_j);
    // This should never be called on a hit result with an existing anchor
    passert (hit->getAnchor() == nullptr);

    std::shared_ptr<VROARNode> node = std::make_shared<VROARNode>();
    std::shared_ptr<VROARAnchor> anchor = hit->createAnchorAtHitLocation(node);
    return VRO_REF_NEW(VROARNode, node);
}

VRO_METHOD(void, nativeDestroyARHitTestResult)(VRO_ARGS
                                               VRO_REF(VROARHitTestResultARCore) hit_j) {
    VRO_REF_DELETE(VROARHitTestResultARCore, hit_j);
}

}