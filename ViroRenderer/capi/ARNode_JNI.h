#ifndef ANDROID_ARNODE_JNI_H
#define ANDROID_ARNODE_JNI_H

#include <memory>
#include <VROARNode.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARNode {
    inline VRO_REF(VROARNode) jptr(std::shared_ptr<VROARNode> node) {
        PersistentRef<VROARNode> *node_p = new PersistentRef<VROARNode>(node);
        return reinterpret_cast<intptr_t>(node_p);
    }

    inline std::shared_ptr<VROARNode> native(VRO_REF(VROARNode) node_j) {
        PersistentRef<VROARNode> *node_p = reinterpret_cast<PersistentRef<VROARNode> *>(node_j);
        return node_p->get();
    }
}

#endif //ANDROID_ARNODE_JNI_H
