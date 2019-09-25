//
//  VRODualQuaternion.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/23/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VRODualQuaternion.h"
#include "VROLog.h"
VRODualQuaternion::VRODualQuaternion() :
    _real(0, 0, 0, 1),
    _dual(0, 0, 0, 0)
{}

VRODualQuaternion::VRODualQuaternion(VROQuaternion real, VROQuaternion dual) :
    _real(real),
    _dual(dual)
{}

VRODualQuaternion::VRODualQuaternion(VROVector3f translation, VROQuaternion rotation) {
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
