#ifndef ANDROID_ARNODE_JNI_H
#define ANDROID_ARNODE_JNI_H

#include <jni.h>
#include <memory>
#include <VROARNode.h>
#include "PersistentRef.h"

namespace ARNode {
    inline jlong jptr(std::shared_ptr<VROARNode> node) {
        PersistentRef<VROARNode> *node_p = new PersistentRef<VROARNode>(node);
        return reinterpret_cast<intptr_t>(node_p);
    }

    inline std::shared_ptr<VROARNode> native(jlong node_j) {
        PersistentRef<VROARNode> *node_p = reinterpret_cast<PersistentRef<VROARNode> *>(node_j);
        return node_p->get();
    }
}

#endif //ANDROID_ARNODE_JNI_H
