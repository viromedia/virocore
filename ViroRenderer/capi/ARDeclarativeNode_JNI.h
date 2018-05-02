#ifndef ANDROID_ARDECLARATIVENODE_H
#define ANDROID_ARDECLARATIVENODE_H

#include <memory>
#include <VROARDeclarativePlane.h>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARDeclarativeNode {
    inline VRO_REF(VROARDeclarativeNode) jptr(std::shared_ptr<VROARDeclarativeNode> node) {
        PersistentRef<VROARDeclarativeNode> *node_p = new PersistentRef<VROARDeclarativeNode>(node);
        return reinterpret_cast<intptr_t>(node_p);
    }

    inline std::shared_ptr<VROARDeclarativeNode> native(VRO_REF(VROARDeclarativeNode) node_j) {
        PersistentRef<VROARDeclarativeNode> *node_p = reinterpret_cast<PersistentRef<VROARDeclarativeNode> *>(node_j);
        return node_p->get();
    }
}

class ARDeclarativeNodeDelegate : public VROARDeclarativeNodeDelegate {
public:
    ARDeclarativeNodeDelegate(VRO_OBJECT arNodeObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arNodeObject)) {
    }

    ~ARDeclarativeNodeDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    static VRO_REF(ARDeclarativeNodeDelegate) jptr(std::shared_ptr<ARDeclarativeNodeDelegate> delegate) {
        PersistentRef<ARDeclarativeNodeDelegate> *delegate_p = new PersistentRef<ARDeclarativeNodeDelegate>(delegate);
        return reinterpret_cast<intptr_t>(delegate_p);
    }

    static std::shared_ptr<ARDeclarativeNodeDelegate> native(VRO_REF(ARDeclarativeNodeDelegate) delegate_j) {
        PersistentRef<ARDeclarativeNodeDelegate> *delegate_p = reinterpret_cast<PersistentRef<ARDeclarativeNodeDelegate> *>(delegate_j);
        return delegate_p->get();
    }

    void onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorRemoved();

private:
    VRO_OBJECT _javaObject;
};

#endif //ANDROID_ARDECLARATIVENODE_H
