//
//  VROFrustum.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROFrustum.h"
#include "VROFrustumPlane.h"
#include "VROMath.h"
#include <limits>

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialization
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialization

VROFrustum::VROFrustum() {
    
}

VROFrustum::~VROFrustum() {

}

void VROFrustum::fitToModelView(const float *view, const float *projection,
                                const float bufferSides, const float bufferNear, const float bufferFar) {

    float mtx[16];
    VROMathMultMatrices(view, projection, mtx);

    /* Left plane */
    VROFrustumPlane &left = _planes[(int)VROFrustumSide::Left];
    left.normal.x = (mtx[3] + mtx[0]);
    left.normal.y = (mtx[7] + mtx[4]);
    left.normal.z = (mtx[11] + mtx[8]);
    left.d = (mtx[15] + mtx[12]);

    left.normalize();
    left.refreshFarPoints();

    /* Right plane */
    VROFrustumPlane &right = _planes[(int)VROFrustumSide::Right];
    right.normal.x = (mtx[3] - mtx[0]);
    right.normal.y = (mtx[7] - mtx[4]);
    right.normal.z = (mtx[11] - mtx[8]);
    right.d = (mtx[15] - mtx[12]);

    right.normalize();
    right.refreshFarPoints();

    /* Bottom plane */
    VROFrustumPlane &bottom = _planes[(int)VROFrustumSide::Bottom];
    bottom.normal.x = (mtx[3] + mtx[1]);
    bottom.normal.y = (mtx[7] + mtx[5]);
    bottom.normal.z = (mtx[11] + mtx[9]);
    bottom.d = (mtx[15] + mtx[13]);

    bottom.normalize();
    bottom.refreshFarPoints();

    /* Top plane */
    VROFrustumPlane &top = _planes[(int)VROFrustumSide::Top];
    top.normal.x = (mtx[3] - mtx[1]);
    top.normal.y = (mtx[7] - mtx[5]);
    top.normal.z = (mtx[11] - mtx[9]);
    top.d = (mtx[15] - mtx[13]);

    top.normalize();
    top.refreshFarPoints();

    /* Near plane */
    VROFrustumPlane &near = _planes[(int)VROFrustumSide::Near];
    near.normal.x = (mtx[3] + mtx[2]);
    near.normal.y = (mtx[7] + mtx[6]);
    near.normal.z = (mtx[11] + mtx[10]);
    near.d = (mtx[15] + mtx[14]);

    near.normalize();
    near.refreshFarPoints();

    /* Far plane */
    VROFrustumPlane &far = _planes[(int)VROFrustumSide::Far];
    far.normal.x = (mtx[3] - mtx[2]);
    far.normal.y = (mtx[7] - mtx[6]);
    far.normal.z = (mtx[11] - mtx[10]);
    far.d = (mtx[15] - mtx[14]);

    far.normalize();
    far.refreshFarPoints();

    /*
     Expand the frustum slightly to mitigate popping. Make negative to contract.
     */
    _planes[(int)VROFrustumSide::Right].d += bufferSides;
    _planes[(int)VROFrustumSide::Left].d += bufferSides;
    _planes[(int)VROFrustumSide::Top].d += bufferSides;
    _planes[(int)VROFrustumSide::Bottom].d += bufferSides;
    _planes[(int)VROFrustumSide::Near].d += bufferNear;
    _planes[(int)VROFrustumSide::Far].d += bufferFar;
}

void VROFrustum::removeFCP() {
    _planes[(int)VROFrustumSide::Far].d = std::numeric_limits<float>::max();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Intersection Testing
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Intersection Testing

VROFrustumResult VROFrustum::intersectAllOpt(const VROBoundingBox &box, VROFrustumBoxIntersectionMetadata *metadata) const {
    const float *boxPlanes = box.getPlanes();
    int startingPlane = metadata->getPlaneLastOutside();

    bool intersect = false;

    /*
     First check the plane that this box was outside of during the last
     frame, for temporal coherency optimization. If the box was inside the
     frustum in the last frame then this optimization does nothing.
     */
    const VROFrustumPlane *plane = &_planes[startingPlane];
    const VROVector3f *normal = &plane->normal;
    float d = plane->d;

    const VROBoxPlane *farPoints = plane->farPoints;
    float distanceToInnerPoint = normal->x * boxPlanes[farPoints[VROFarPointPosX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointPosY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointPosZ]]
                               + d;

    if (distanceToInnerPoint < 0) {
        return VROFrustumResult::Outside;
    }

    float distanceToOuterPoint = normal->x * boxPlanes[farPoints[VROFarPointNegX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointNegY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointNegZ]]
                               + d;
    if (distanceToOuterPoint < 0) {
        intersect = true;
    }

    /*
     Now test the remaining planes.
     */
    for (int i = 0; i < 6; i++) {
        if (i == startingPlane) {
            continue;
        }

        plane = &_planes[i];
        normal = &plane->normal;
        d = plane->d;
        farPoints = plane->farPoints;

        /*
         If the positive far point is outside any plane, then we know that
         the whole box is outside of the entire frustum, so we can break
         out and return. Store this plane so that we check it first next
         time.
         */
        distanceToInnerPoint = normal->x * boxPlanes[farPoints[VROFarPointPosX]]
                             + normal->y * boxPlanes[farPoints[VROFarPointPosY]]
                             + normal->z * boxPlanes[farPoints[VROFarPointPosZ]]
                             + d;
        if (distanceToInnerPoint < 0) {
            metadata->setPlaneLastOutside(i);
            return VROFrustumResult::Outside;
        }

        /*
         If the negative far point is outside the frustum, then we know we
         can't be inside, so set intersect=true. We continue iterating,
         however, because we may still be outside.
         */
        distanceToOuterPoint = normal->x * boxPlanes[farPoints[VROFarPointNegX]]
                             + normal->y * boxPlanes[farPoints[VROFarPointNegY]]
                             + normal->z * boxPlanes[farPoints[VROFarPointNegZ]]
                             + d;
        if (distanceToOuterPoint < 0) {
            intersect = true;
        }
    }

    /*
     If any negative far point was outside the frustum, then we know we can't be inside.
     */
    if (intersect) {
        return VROFrustumResult::Intersects;
    }
    else {
        return VROFrustumResult::Inside;
    }
}

VROFrustumResult VROFrustum::intersectWithFarPointsOpt(const VROBoundingBox &box) const {
    const float *boxPlanes = box.getPlanes();

    bool isInside = true;

    for (int i = 0; i < 6; i++) {
        const VROVector3f &normal = _planes[i].normal;
        float d = _planes[i].d;

        const VROBoxPlane *farPoints = _planes[i].farPoints;

        /*
         If the positive far point is outside any plane, then we know that whole box is outside
         of the entire frustum, so we can break out and return. Store this plane so that we check
         it first next time.
         */
        float distanceToInnerPoint = normal.x * boxPlanes[farPoints[VROFarPointPosX]]
                                   + normal.y * boxPlanes[farPoints[VROFarPointPosY]]
                                   + normal.z * boxPlanes[farPoints[VROFarPointPosZ]]
                                   + d;
        if (distanceToInnerPoint < 0) {
            return VROFrustumResult::Outside;
        }

        /*
         If the negative far point is outside the frustum, then we know we can't be inside, so set
         isInside=false. We continue iterating, however, because we may still be outside.
         */
        float distanceToOuterPoint = normal.x * boxPlanes[farPoints[VROFarPointNegX]]
                                   + normal.y * boxPlanes[farPoints[VROFarPointNegY]]
                                   + normal.z * boxPlanes[farPoints[VROFarPointNegZ]]
                                   + d;
        if (distanceToOuterPoint < 0) {
            isInside = false;
        }
    }

    /*
     If any negative far point was outside the frustum, then we know we can't be inside.
     */
    if (isInside) {
        return VROFrustumResult::Inside;
    }
    else {
        return VROFrustumResult::Intersects;
    }
}

VROFrustumResult VROFrustum::intersectNoOpt(const VROBoundingBox &box) const {
    int numPointsContained = 0;

    std::vector<VROVector3f> points {
        {box.getMinX(), box.getMinY(), box.getMinZ()},
        {box.getMaxX(), box.getMinY(), box.getMinZ()},
        {box.getMinX(), box.getMaxY(), box.getMinZ()},
        {box.getMaxX(), box.getMaxY(), box.getMinZ()},
        {box.getMinX(), box.getMinY(), box.getMaxZ()},
        {box.getMaxX(), box.getMinY(), box.getMaxZ()},
        {box.getMinX(), box.getMaxY(), box.getMaxZ()},
        {box.getMaxX(), box.getMaxY(), box.getMaxZ()}
    };

    for (size_t j = 0; j < points.size(); j++) {
        int numOutside = 0;

        for (int i = 0; i < 6; i++) {
            const VROVector3f &normal = _planes[i].normal;
            float d = _planes[i].d;

            float distanceToInnerPoint = normal.x * points[j].x + normal.y * points[j].y + d;
            if (distanceToInnerPoint < 0) {
                ++numOutside;
            }
        }

        if (numOutside == 0) {
            ++numPointsContained;
        }
    }

    if (numPointsContained == 8) {
        return VROFrustumResult::Inside;
    }
    else if (numPointsContained == 0) {
        return VROFrustumResult::Outside;
    }
    else {
        return VROFrustumResult::Intersects;
    }
}

bool VROFrustum::containsPoint(const VROVector3f &pt) const {
    for (int i = 0; i < 6; i++) {
        if (_planes[i].distanceToPoint(pt) < 0) {
            return false;
        }
    }
    
    return true;
}

float VROFrustum::distanceFromFCP(VROVector3f pt) const {
    return _planes[(int)VROFrustumSide::Far].distanceToPoint(pt);
}

float VROFrustum::distanceFromNCP(VROVector3f pt) const {
    return _planes[(int)VROFrustumSide::Near].distanceToPoint(pt);
}

VROFrustumResult VROFrustum::intersect(const VROBoundingBox &box, VROFrustumBoxIntersectionMetadata *metadata) const {
    if (!metadata) {
        return intersectWithFarPointsOpt(box);
    }
    else {
        return intersectAllOpt(box, metadata);
    }
}
