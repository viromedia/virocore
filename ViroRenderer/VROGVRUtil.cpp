//
//  VROGVRUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/11/17
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROGVRUtil.h"

VROMatrix4f VROGVRUtil::toMatrix4f(const gvr::Mat4f &glm) {
    float m[16] = {
            glm.m[0][0], glm.m[1][0], glm.m[2][0], glm.m[3][0],
            glm.m[0][1], glm.m[1][1], glm.m[2][1], glm.m[3][1],
            glm.m[0][2], glm.m[1][2], glm.m[2][2], glm.m[3][2],
            glm.m[0][3], glm.m[1][3], glm.m[2][3], glm.m[3][3],
    };

    return VROMatrix4f(m);
}

gvr::Mat4f VROGVRUtil::toGVRMat4f(VROMatrix4f m) {
    gvr::Mat4f glm;
    glm.m[0][0] = m[0];
    glm.m[1][0] = m[1];
    glm.m[2][0] = m[2];
    glm.m[3][0] = m[3];
    glm.m[0][1] = m[4];
    glm.m[1][1] = m[5];
    glm.m[2][1] = m[6];
    glm.m[3][1] = m[7];
    glm.m[0][2] = m[8];
    glm.m[1][2] = m[9];
    glm.m[2][2] = m[10];
    glm.m[3][2] = m[11];
    glm.m[0][3] = m[12];
    glm.m[1][3] = m[13];
    glm.m[2][3] = m[14];
    glm.m[3][3] = m[15];

    return glm;
}

