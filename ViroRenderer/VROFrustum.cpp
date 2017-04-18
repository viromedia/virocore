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

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialization
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialization

VROFrustum::VROFrustum(bool tdf) :
    frame(0U),
    tdf(tdf) {
}

VROFrustum::~VROFrustum() {

}

void VROFrustum::fitToModelView(const double *view, const float *projection,
        const float bufferSides, const float bufferNear, const float bufferFar) {

    double mtx[16];
    VROMathMultMatrices_dfd(view, projection, mtx);

    /* Left plane */
    VROFrustumPlane &left = planes[(int)VROFrustumSide::Left];
    left.normal.x = (mtx[3] + mtx[0]);
    left.normal.y = (mtx[7] + mtx[4]);
    left.normal.z = (mtx[11] + mtx[8]);
    left.d = (mtx[15] + mtx[12]);

    left.normalize();
    left.refreshFarPoints();

    /* Right plane */
    VROFrustumPlane &right = planes[(int)VROFrustumSide::Right];
    right.normal.x = (mtx[3] - mtx[0]);
    right.normal.y = (mtx[7] - mtx[4]);
    right.normal.z = (mtx[11] - mtx[8]);
    right.d = (mtx[15] - mtx[12]);

    right.normalize();
    right.refreshFarPoints();

    /* Bottom plane */
    VROFrustumPlane &bottom = planes[(int)VROFrustumSide::Bottom];
    bottom.normal.x = (mtx[3] + mtx[1]);
    bottom.normal.y = (mtx[7] + mtx[5]);
    bottom.normal.z = (mtx[11] + mtx[9]);
    bottom.d = (mtx[15] + mtx[13]);

    bottom.normalize();
    bottom.refreshFarPoints();

    /* Top plane */
    VROFrustumPlane &top = planes[(int)VROFrustumSide::Top];
    top.normal.x = (mtx[3] - mtx[1]);
    top.normal.y = (mtx[7] - mtx[5]);
    top.normal.z = (mtx[11] - mtx[9]);
    top.d = (mtx[15] - mtx[13]);

    top.normalize();
    top.refreshFarPoints();

    /* Near plane */
    VROFrustumPlane &near = planes[(int)VROFrustumSide::Near];
    near.normal.x = (mtx[3] + mtx[2]);
    near.normal.y = (mtx[7] + mtx[6]);
    near.normal.z = (mtx[11] + mtx[10]);
    near.d = (mtx[15] + mtx[14]);

    near.normalize();
    near.refreshFarPoints();

    /* Far plane */
    VROFrustumPlane &far = planes[(int)VROFrustumSide::Far];
    far.normal.x = (mtx[3] - mtx[2]);
    far.normal.y = (mtx[7] - mtx[6]);
    far.normal.z = (mtx[11] - mtx[10]);
    far.d = (mtx[15] - mtx[14]);

    far.normalize();
    far.refreshFarPoints();

    /*
     * Expand the frustum slightly to mitigate popping. Make negative to contract.
     */
    planes[(int)VROFrustumSide::Right].d += bufferSides;
    planes[(int)VROFrustumSide::Left].d += bufferSides;
    planes[(int)VROFrustumSide::Top].d += bufferSides;
    planes[(int)VROFrustumSide::Bottom].d += bufferSides;

    planes[(int)VROFrustumSide::Near].d += bufferNear;
    planes[(int)VROFrustumSide::Far].d += bufferFar;
}

void VROFrustum::fitToFrustum(const VROFrustum &source, const VROVector3f &distance) {
    for (int i = 0; i < 6; i++) {
        planes[i] = source.planes[i];
    }
    frame = source.frame;

    bool copyingFromTDF = source.tdf;
    if (copyingFromTDF) {
        delta.x = source.delta.x;
        delta.y = source.delta.y;
        delta.z = source.delta.z;

        for (int i = 0; i < 6; i++) {
            planeDeltas[i] = source.planeDeltas[i];
        }
    }

    else {
        delta.x = 0;
        delta.y = 0;
        delta.z = 0;

        for (int i = 0; i < 6; i++) {
            planeDeltas[i] = 0;
        }
    }

    delta.x -= distance.x;
    delta.y -= distance.y;
    delta.z -= distance.z;

    for (int i = 0; i < 6; i++) {
        VROPlane &plane = planes[i];
        float pdelta = plane.normal.dot(distance);

        planeDeltas[i] += pdelta;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Intersection Testing
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Intersection Testing

VROFrustumResult VROFrustum::intersectTDFUnoptimized(VROBoundingBox *box, VROFrustumBoxIntersectionMetadata *metadata) const {
    uint32_t frame = this->frame;

    const float *boxPlanes = box->getPlanes();
    int startingPlane = metadata->getPlaneLastOutside();

    /*
     As we perform intersection we record all distances here for use with the translation-coherency
     optimization.
     */
    float *posDistances = metadata->getPositiveDistanceFromPlanes();
    float *negDistances = metadata->getNegativeDistanceFromPlanes();

    /*
     Set the frame so that we know to what frustum the recorded positive and negative distances
     will apply.
     */
    metadata->setDistanceFrame(frame);
    metadata->setSourceFrustumForDistances(this);

    bool intersect = false;

    /*
     First check the plane that this box was outside of during the last frame, for temporal coherency
     optimization. If the box was inside the frustum in the last frame then this optimization does nothing.
     */
    const VROFrustumPlane *plane = &planes[startingPlane];
    const VROVector3f *normal = &plane->normal;
    float d = plane->d;

    const VROBoxPlane *farPoints = plane->farPoints;

    float distanceToInnerPoint = normal->x * boxPlanes[farPoints[VROFarPointPosX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointPosY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointPosZ]]
                               + d;
    posDistances[startingPlane] = distanceToInnerPoint;

    if (distanceToInnerPoint + planeDeltas[startingPlane] < 0) {
        metadata->setFrustumDistanceValid(false);
        return VROFrustumResult::Outside;
    }

    float distanceToOuterPoint = normal->x * boxPlanes[farPoints[VROFarPointNegX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointNegY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointNegZ]]
                               + d;
    negDistances[startingPlane] = distanceToOuterPoint;

    if (distanceToOuterPoint + planeDeltas[startingPlane] < 0) {
        intersect = true;
    }

    /*
     Now test the remaining planes.
     */
    for (int i = 0; i < 6; i++) {
        if (i == startingPlane) {
            continue;
        }

        plane = &planes[i];
        normal = &plane->normal;
        d = plane->d;
        farPoints = plane->farPoints;

        /*
         If the positive far point is outside any plane, then we know that the whole box is outside
         of the entire frustum, so we can break out and return. Store this plane so that we check it
         first next time.
         */
        distanceToInnerPoint = normal->x * boxPlanes[farPoints[VROFarPointPosX]]
                             + normal->y * boxPlanes[farPoints[VROFarPointPosY]]
                             + normal->z * boxPlanes[farPoints[VROFarPointPosZ]]
                             + d;
        posDistances[i] = distanceToInnerPoint;
        if (distanceToInnerPoint + planeDeltas[i] < 0) {
            metadata->setPlaneLastOutside(i);
            metadata->setFrustumDistanceValid(false);

            return VROFrustumResult::Outside;
        }

        /*
         If the negative far point is outside the frustum, then we know we can't be inside, so set
         intersect=true. We continue iterating, however, because we may still be outside.
         */
        distanceToOuterPoint = normal->x * boxPlanes[farPoints[VROFarPointNegX]]
                             + normal->y * boxPlanes[farPoints[VROFarPointNegY]]
                             + normal->z * boxPlanes[farPoints[VROFarPointNegZ]]
                             + d;
        negDistances[i] = distanceToOuterPoint;
        if (distanceToOuterPoint + planeDeltas[i] < 0) {
            intersect = true;
        }
    }

    /*
     If we made it here then the distances of the box from all six planes have been recorded, so mark
     this as true.
     */
    metadata->setFrustumDistanceValid(true);

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

VROFrustumResult VROFrustum::intersectTDF(VROBoundingBox *box, VROFrustumBoxIntersectionMetadata *metadata) const {
    const float *planeDeltas = this->planeDeltas;
    uint32_t frame = this->frame;

    int startingPlane = metadata->getPlaneLastOutside();
    float *posDistances = metadata->getPositiveDistanceFromPlanes();
    float *negDistances = metadata->getNegativeDistanceFromPlanes();

    /*
     If the box has distances recorded for a frustum other than the source of this frustum, then
     we have to fallback to intersectUnoptimized. First, however, set the box's distances as
     invalid (since it has old distances).
     */
    if (metadata->getDistanceFrame() != frame || metadata->getSourceFrustumForDistances() != this) {
        metadata->setFrustumDistanceValid(false);
        return intersectTDFUnoptimized(box, metadata);
    }

    bool intersect = false;

    /*
     First check the plane that this box was outside of during the last frame, for temporal coherency
     optimization. If the box was inside the frustum in the last frame then this optimization does
     nothing.
     */
    float posDistanceStart = posDistances[startingPlane];
    float planeDeltaStart = planeDeltas[startingPlane];
    if (posDistanceStart + planeDeltaStart < 0) {
        return VROFrustumResult::Outside;
    }

    /*
     If the rest of the box's distances are not recorded, then we have to fallback to intersect unoptimized.
     We perform this check here instead of at the top of this function because the distance from the starting
     plane is ALWAYS recorded.
     */
    if (!metadata->isFrustumDistanceValid()) {
        return intersectTDFUnoptimized(box, metadata);
    }

    if (negDistances[startingPlane] + planeDeltaStart < 0) {
        intersect = true;
    }

    /*
     Now test the remaining planes. If we already intersected, we don't have to test for any further
     intersections.
     */
    if (!intersect) {
        for (int i = 0; i < 6; i++) {
            if (i == startingPlane) {
                continue;
            }

            /*
             If the positive far point is outside any plane, then we know that
             the whole box is outside of the entire frustum, so we can break
             out and return. Store this plane so that we check it first next
             time.
             */
            if (posDistances[i] + planeDeltas[i] < 0) {
                metadata->setPlaneLastOutside(i);
                return VROFrustumResult::Outside;
            }

            /*
             If the negative far point is outside the frustum, then we know we
             can't be inside, so set intersect=true. We continue iterating,
             however, because we may still be outside.
             */
            if (!intersect && negDistances[i] + planeDeltas[i] < 0) {
                intersect = true;
            }
        }

        /*
         If any negative far point was outside the frustum, then we know we
         can't be inside.
         */
        if (intersect) {
            return VROFrustumResult::Intersects;
        }
        else {
            return VROFrustumResult::Inside;
        }
    }
    else {
        for (int i = 0; i < 6; i++) {
            if (i == startingPlane) {
                continue;
            }

            if (posDistances[i] + planeDeltas[i] < 0) {
                metadata->setPlaneLastOutside(i);
                return VROFrustumResult::Outside;
            }
        }

        return VROFrustumResult::Intersects;
    }
}

VROFrustumResult VROFrustum::intersectGeneric(VROBoundingBox *box, VROFrustumBoxIntersectionMetadata *metadata) const {
    const float *boxPlanes = box->getPlanes();
    int startingPlane = metadata->getPlaneLastOutside();

    /*
     As we perform intersection we record all distances here for use with
     the translation-coherency optimization.
     */
    float *posDistances = metadata->getPositiveDistanceFromPlanes();
    float *negDistances = metadata->getNegativeDistanceFromPlanes();

    /*
     Set the frame so that we know to what frustum the recorded positive
     and negative distances will apply.
     */
    metadata->setDistanceFrame(frame);
    metadata->setSourceFrustumForDistances(this);

    bool intersect = false;

    /*
     First check the plane that this box was outside of during the last
     frame, for temporal coherency optimization. If the box was inside the
     frustum in the last frame then this optimization does nothing.
     */
    const VROFrustumPlane *plane = &planes[startingPlane];
    const VROVector3f *normal = &plane->normal;
    float d = plane->d;

    const VROBoxPlane *farPoints = plane->farPoints;
    float distanceToInnerPoint = normal->x * boxPlanes[farPoints[VROFarPointPosX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointPosY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointPosZ]]
                               + d;
    posDistances[startingPlane] = distanceToInnerPoint;

    if (distanceToInnerPoint < 0) {
        metadata->setFrustumDistanceValid(false);
        return VROFrustumResult::Outside;
    }

    float distanceToOuterPoint = normal->x * boxPlanes[farPoints[VROFarPointNegX]]
                               + normal->y * boxPlanes[farPoints[VROFarPointNegY]]
                               + normal->z * boxPlanes[farPoints[VROFarPointNegZ]]
                               + d;
    negDistances[startingPlane] = distanceToOuterPoint;
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

        plane = &planes[i];
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
        posDistances[i] = distanceToInnerPoint;
        if (distanceToInnerPoint < 0) {
            metadata->setPlaneLastOutside(i);
            metadata->setFrustumDistanceValid(false);

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
        negDistances[i] = distanceToOuterPoint;
        if (distanceToOuterPoint < 0) {
            intersect = true;
        }
    }

    /*
     If we made it here then the distances of the box from all six planes
     have been recorded, so mark this as true.
     */
    metadata->setFrustumDistanceValid(true);

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

VROFrustumResult VROFrustum::intersectWithFarPointsOpt(VROBoundingBox *box, VROFrustumBoxIntersectionMetadata *metadata) const {
    const float *boxPlanes = box->getPlanes();

    bool isInside = true;

    for (int i = 0; i < 4; i++) {
        const VROVector3f &normal = planes[i].normal;
        float d = planes[i].d;

        const VROBoxPlane *farPoints = planes[i].farPoints;

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

VROFrustumResult VROFrustum::intersectNoOpt(VROBoundingBox *box) const {
    int numPointsContained = 0;

    std::vector<VROVector3f> points {
        {box->getMinX(), box->getMinY(), box->getMinZ()},
        {box->getMaxX(), box->getMinY(), box->getMinZ()},
        {box->getMinX(), box->getMaxY(), box->getMinZ()},
        {box->getMaxX(), box->getMaxY(), box->getMinZ()},
        {box->getMinX(), box->getMinY(), box->getMaxZ()},
        {box->getMaxX(), box->getMinY(), box->getMaxZ()},
        {box->getMinX(), box->getMaxY(), box->getMaxZ()},
        {box->getMaxX(), box->getMaxY(), box->getMaxZ()}
    };

    for (size_t j = 0; j < points.size(); j++) {
        int numOutside = 0;

        for (int i = 0; i < 6; i++) {
            const VROVector3f &normal = planes[i].normal;
            float d = planes[i].d;

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

VROFrustumResult VROFrustum::intersectSlow(VROBoundingBox *box) const {
    std::vector<VROVector3f> points {
        {box->getMinX(), box->getMinY(), box->getMinZ()},
        {box->getMaxX(), box->getMinY(), box->getMinZ()},
        {box->getMinX(), box->getMaxY(), box->getMinZ()},
        {box->getMaxX(), box->getMaxY(), box->getMinZ()},
        {box->getMinX(), box->getMinY(), box->getMaxZ()},
        {box->getMaxX(), box->getMinY(), box->getMaxZ()},
        {box->getMinX(), box->getMaxY(), box->getMaxZ()},
        {box->getMaxX(), box->getMaxY(), box->getMaxZ()}
    };

    bool intersects = false;
    bool contains = true;

    for (size_t i = 0; i < points.size(); i++) {
        if (containsPoint(points[i])) {
            intersects = true;
        }
        else {
            contains = false;
        }
    }

    /*
     If any negative far point was outside the frustum, then we know we can't be inside.
     */
    if (contains) {
        return VROFrustumResult::Inside;
    }
    else if (intersects) {
        return VROFrustumResult::Intersects;
    }
    else {
        return VROFrustumResult::Outside;
    }
}

bool VROFrustum::containsPoint(const VROVector3f &pt) const {
    if (!tdf) {
        for (int i = 0; i < 6; i++) {
            if (planes[i].distanceToPoint(pt) < 0) {
                return false;
            }
        }

        return true;
    }
    else {
        for (int i = 0; i < 6; i++) {
            if (planes[i].distanceToPoint(pt) + planeDeltas[i] < 0) {
                return false;
            }
        }

        return true;
    }
}

float VROFrustum::distanceFromFCP(VROVector3f pt) const {
    if (!tdf) {
        return planes[(int)VROFrustumSide::Far].distanceToPoint(pt);
    }
    else {
        return planes[(int)VROFrustumSide::Far].distanceToPoint(pt) + planeDeltas[(int)VROFrustumSide::Far];
    }
}

float VROFrustum::distanceFromNCP(VROVector3f pt) const {
    if (!tdf) {
        return planes[(int)VROFrustumSide::Near].distanceToPoint(pt);
    }
    else {
        return planes[(int)VROFrustumSide::Near].distanceToPoint(pt) + planeDeltas[(int)VROFrustumSide::Near];
    }
}

VROFrustumResult VROFrustum::intersect(VROBoundingBox *box, VROFrustumBoxIntersectionMetadata *metadata) const {
    if (tdf) {
        return intersectTDF(box, metadata);
    }
    else {
        return intersectGeneric(box, metadata);
    }
}
