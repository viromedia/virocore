//
//  TransformDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef TransformDelegate_JNI_H
#define TransformDelegate_JNI_H

#include <VROTransformDelegate.h>
#include "VROVideoDelegateInternal.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class TransformDelegate_JNI : public VROTransformDelegate {
public:
    TransformDelegate_JNI(VRO_OBJECT delegateJavaObject, double distanceFilter);
    ~TransformDelegate_JNI();

    /*
     Notification delegate to let the bridge know that the position has changed.
     */
    void onPositionUpdate(VROVector3f position);

private:
    VRO_OBJECT _javaObject;
};
#endif
