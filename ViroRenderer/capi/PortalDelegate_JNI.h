/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */


#ifndef ANDROID_PORTALDELEGATE_JNI_H
#define ANDROID_PORTALDELEGATE_JNI_H

#include "VROPortalDelegate.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class PortalDelegate: public VROPortalDelegate {
    public:
    PortalDelegate(VRO_OBJECT javaObject);
    ~PortalDelegate();

    virtual void onPortalEnter();
    virtual void onPortalExit();
private:
    VRO_OBJECT _javaObject;
};


#endif //ANDROID_PORTALDELEGATE_JNI_H
