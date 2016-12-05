//
//  Node_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VRONode.h>

#include "PersistentRef.h"

namespace Node{
    inline jlong jptr(std::shared_ptr<VRONode> shared_node) {
        PersistentRef<VRONode> *native_node = new PersistentRef<VRONode>(shared_node);
        return reinterpret_cast<intptr_t>(native_node);
    }

    inline std::shared_ptr<VRONode> native(jlong ptr) {
        PersistentRef<VRONode> *persistentNode = reinterpret_cast<PersistentRef<VRONode> *>(ptr);
        return persistentNode->get();
    }
}