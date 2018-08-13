#include <VROPlatformUtil.h>
#include <arcore/VROARAnchorARCore.h>
#include "ARNode_JNI.h"
#include "ViroUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#include "arcore/ARUtils_JNI.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARNode_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARNode_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeSetPauseUpdates)(VRO_ARGS
                                        VRO_REF(VROARNode) node_j,
                                        VRO_BOOL pauseUpdates) {
    std::weak_ptr<VROARNode> node_w = VRO_REF_GET(VROARNode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w, pauseUpdates] {
        std::shared_ptr<VROARNode> node = node_w.lock();
        node->setPauseUpdates(pauseUpdates);
    });
}

VRO_METHOD(VRO_OBJECT, nativeGetAnchor)(VRO_ARGS
                                        VRO_REF(VROARNode) node_j) {
#ifdef VRO_PLATFORM_ANDROID
    std::shared_ptr<VROARNode> node = VRO_REF_GET(VROARNode, node_j);
    std::shared_ptr<VROARAnchor> anchor = node->getAnchor();
    return ARUtilsCreateJavaARAnchorFromAnchor(anchor);
#else
    return nullptr;
#endif
}

VRO_METHOD(void, nativeDetach)(VRO_ARGS
                               VRO_REF(VROARNode) node_j) {
    std::weak_ptr<VROARNode> node_w = VRO_REF_GET(VROARNode, node_j);

    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VROARNode> node = node_w.lock();
        if (!node) {
            return;
        }
        std::shared_ptr<VROARAnchorARCore> anchor = std::dynamic_pointer_cast<VROARAnchorARCore>(node->getAnchor());
        if (anchor) {
            anchor->detach();
        }
    });
}

VRO_METHOD(bool, nativeIsAnchorManaged)(VRO_ARGS
                                        VRO_REF(VROARNode) node_j) {
    std::shared_ptr<VROARNode> node = VRO_REF_GET(VROARNode, node_j);
    std::shared_ptr<VROARAnchorARCore> anchor = std::dynamic_pointer_cast<VROARAnchorARCore>(node->getAnchor());
    passert (anchor);

    return anchor->isManaged();
}

}
