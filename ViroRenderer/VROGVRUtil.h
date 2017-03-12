//
//  VROGVRUtil.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/11/17
//  Copyright © 2017 Viro Media. All rights reserved.
//
#ifndef ANDROID_VROGVRUTIL_H
#define ANDROID_VROGVRUTIL_H

#include "VROMatrix4f.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROGVRUtil {
public:

    static VROMatrix4f toMatrix4f(const gvr::Mat4f &glm);
    static gvr::Mat4f toGVRMat4f(VROMatrix4f matrix);
};


#endif //ANDROID_VROGVRUTIL_H
