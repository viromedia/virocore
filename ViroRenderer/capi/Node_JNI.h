//
//  Node_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef Node_JNI_h
#define Node_JNI_h

#include <memory>
#include "VRONode.h"
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Node {
    inline VRO_REF(VRONode) jptr(std::shared_ptr<VRONode> shared_node) {
        PersistentRef<VRONode> *native_node = new PersistentRef<VRONode>(shared_node);
        return reinterpret_cast<intptr_t>(native_node);
    }

    inline std::shared_ptr<VRONode> native(VRO_REF(VRONode) ptr) {
        PersistentRef<VRONode> *persistentNode = reinterpret_cast<PersistentRef<VRONode> *>(ptr);
        return persistentNode->get();
    }
}

#endif