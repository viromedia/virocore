//
//  VRODualQuaternion.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/23/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRODualQuaternion.h"

VRODualQuaternion::VRODualQuaternion() :
    _real(0, 0, 0, 1),
    _dual(0, 0, 0, 0)
{}

VRODualQuaternion::VRODualQuaternion(VROQuaternion real, VROQuaternion dual) :
    _real(real),
    _dual(dual)
{}

VRODualQuaternion::VRODualQuaternion(VROMatrix4f matrix) {
    VROVector3f translation = matrix.extractTranslation();
    VROVector3f scale = matrix.extractScale();
    VROQuaternion rotation = matrix.extractRotation(scale);
    
    const float qx = rotation.X, qy = rotation.Y, qz = rotation.Z, qw = rotation.W;
    const float tx = translation.x, ty = translation.y, tz = translation.z;
    
    _real = VROQuaternion(qx, qy, qz, qw);
    _dual = VROQuaternion(0.5f * ( tx * qw + ty * qz - tz * qy),
                          0.5f * (-tx * qz + ty * qw + tz * qx),
                          0.5f * ( tx * qy - ty * qx + tz * qw),
                          -0.5f * ( tx * qx + ty * qy + tz * qz));
}

VRODualQuaternion::VRODualQuaternion(VROVector3f translation, VROQuaternion rotation, VROVector3f scale) {
    const float qx = rotation.X, qy = rotation.Y, qz = rotation.Z, qw = rotation.W;
    const float tx = translation.x, ty = translation.y, tz = translation.z;
    
    _real = VROQuaternion(qx, qy, qz, qw);
    _dual = VROQuaternion(0.5f * ( tx * qw + ty * qz - tz * qy),
                          0.5f * (-tx * qz + ty * qw + tz * qx),
                          0.5f * ( tx * qy - ty * qx + tz * qw),
                         -0.5f * ( tx * qx + ty * qy + tz * qz));
}

VRODualQuaternion VRODualQuaternion::operator* (const VRODualQuaternion& right) const {
    return VRODualQuaternion(_real * right._real, _real * right._dual + _dual * right._real);
}

VRODualQuaternion VRODualQuaternion::operator* (float c) const {
    return VRODualQuaternion(_real * c, _dual * c);
}

void VRODualQuaternion::normalize() {
    float realNorm = _real.getNorm();
    if (realNorm > 0) {
        VROQuaternion realN = _real * (1.f / realNorm);
        VROQuaternion dualN = _dual * (1.f / realNorm);
        
        _real = realN;
        _dual = dualN - realN * realN.dotProduct(dualN);
    }
}
