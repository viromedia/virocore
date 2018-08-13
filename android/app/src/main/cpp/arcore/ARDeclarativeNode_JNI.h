#ifndef ANDROID_ARDECLARATIVENODE_H
#define ANDROID_ARDECLARATIVENODE_H

#include <memory>
#include <VROARDeclarativePlane.h>
#include <VROPlatformUtil.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class ARDeclarativeNodeDelegate : public VROARDeclarativeNodeDelegate {
public:
    ARDeclarativeNodeDelegate(VRO_OBJECT arNodeObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arNodeObject)) {
    }

    ~ARDeclarativeNodeDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorRemoved();

private:
    VRO_OBJECT _javaObject;
};

#endif //ANDROID_ARDECLARATIVENODE_H
