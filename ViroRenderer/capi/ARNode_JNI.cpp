#include <VROPlatformUtil.h>
#include "ARNode_JNI.h"
#include "ARUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARNode_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeSetPauseUpdates)(VRO_ARGS
                                        VRO_REF node_j,
                                        VRO_BOOL pauseUpdates) {
    std::weak_ptr<VROARNode> node_w = ARNode::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w, pauseUpdates] {
        std::shared_ptr<VROARNode> node = node_w.lock();
        node->setPauseUpdates(pauseUpdates);
    });
}

}
