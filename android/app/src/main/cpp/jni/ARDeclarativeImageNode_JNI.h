//
//  ARDeclarativeImageNode_JNI.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_ARDECLARATIVEIMAGENODE_JNI_H
#define ANDROID_ARDECLARATIVEIMAGENODE_JNI_H

#include <VROARDeclarativeImageNode.h>
#include <PersistentRef.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARDeclarativeImageNode {
    inline VRO_REF jptr(std::shared_ptr<VROARDeclarativeImageNode> shared_ar_image) {
        PersistentRef<VROARDeclarativeImageNode> *native_ar_image = new PersistentRef<VROARDeclarativeImageNode>(shared_ar_image);
        return reinterpret_cast<intptr_t>(native_ar_image);
    }

    inline std::shared_ptr<VROARDeclarativeImageNode> native(VRO_REF ptr) {
        PersistentRef<VROARDeclarativeImageNode> *persistentARImageNode = reinterpret_cast<PersistentRef<VROARDeclarativeImageNode> *>(ptr);
        return persistentARImageNode->get();
    }
};


#endif //ANDROID_ARDECLARATIVEIMAGENODE_JNI_H
