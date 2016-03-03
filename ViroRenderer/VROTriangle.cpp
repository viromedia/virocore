//
//  VROTriangle.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTriangle.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROLog.h"

VROTriangle::VROTriangle(VROVector3f a, VROVector3f b, VROVector3f c) :
    _a(a),
    _b(b),
    _c(c) {
   
    _segAB = _b.subtract(_a);
    _segBC = _c.subtract(_b);
    _segCA = _a.subtract(_c);
        
    _normal = _segAB.cross(_segBC);
    _normal = _normal.normalize();
}

VROTriangle::~VROTriangle() {

}

bool VROTriangle::isDegenerate() const {
    return _a.isEqual(_b) || _a.isEqual(_c) || _b.isEqual(_c);
}

VROVector3f VROTriangle::vertexWithIndex(int index) const {
    switch (index) {
    case 0:
        return _a;
    case 1:
        return _b;
    case 2:
        return _c;
    default:
        break;
    }

    return _a;
}

VROVector3f VROTriangle::barycenter() const {
    VROVector3f vector;
    vector.x = (_a.x + _b.x + _c.x) / 3;
    vector.y = (_a.y + _b.y + _c.y) / 3;
    vector.z = (_a.z + _b.z + _c.z) / 3;
    
    return vector;
}

bool VROTriangle::intersectsRay(VROVector3f r, VROVector3f origin, VROVector3f *intPt) const {
    if (r.rayIntersectPlane(_a, _normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    return false;
}

bool VROTriangle::containsPoint(VROVector3f p) const {
    //Check if on same side of BC as A (subtract B from both)
    VROVector3f ray = p.subtract(_b);
    VROVector3f tri = _a.subtract(_b);

    VROVector3f rayNormal = _segBC.cross(ray);
    VROVector3f triNormal = _segBC.cross(tri);

    bool sameSideCorner1 = rayNormal.dot(triNormal) >= 0;
    if (!sameSideCorner1) {
        return false;
    }

    //Check if on same side of CA as B (subtract C from both)
    ray = p.subtract(_c);
    tri = _b.subtract(_c);

    rayNormal = _segCA.cross(ray);
    triNormal = _segCA.cross(tri);

    bool sameSideCorner2 = rayNormal.dot(triNormal) >= 0;
    if (!sameSideCorner2) {
        return false;
    }

    //Check if on same side of AB as C (subtract A from both)
    ray = p.subtract(_a);
    tri = _c.subtract(_a);

    rayNormal = _segAB.cross(ray);
    triNormal = _segAB.cross(tri);

    return rayNormal.dot(triNormal) >= 0;
}

VROTriangle VROTriangle::transformByMatrix(VROMatrix4f mtx) const {
    VROVector3f dA = mtx.multiply(_a);
    VROVector3f dB = mtx.multiply(_b);
    VROVector3f dC = mtx.multiply(_c);

    return { dA, dB, dC };
}

