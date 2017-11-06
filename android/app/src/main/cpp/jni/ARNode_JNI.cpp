#include <VROPlatformUtil.h>
#include "ARNode_JNI.h"
#include "ARUtils_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ARNode_##method_name

extern "C" {

JNI_METHOD(void, nativeSetPauseUpdates)(JNIEnv *env,
                                        jobject object,
                                        jlong node_j,
                                        jboolean pauseUpdates) {
    std::weak_ptr<VROARNode> node_w = ARNode::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w, pauseUpdates] {
        std::shared_ptr<VROARNode> node = node_w.lock();
        node->setPauseUpdates(pauseUpdates);
    });
}

}
