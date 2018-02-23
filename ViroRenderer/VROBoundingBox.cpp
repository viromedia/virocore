//
//  VROBoundingBox.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROBoundingBox.h"
#include <sstream>
#include <limits>
#include "VROVector3f.h"
#include <float.h>

//Epsilon value for point containment to account for precision errors
#define kContainsPointEpsilon 0.01

// Uncomment to see a compiler error indicating the size of each VROBoundingBox
// template<int s> struct BoundingBoxSize;
// BoundingBoxSize<sizeof(VROBoundingBox)> boundingBoxSize;

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialization
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialization

VROBoundingBox::VROBoundingBox() :
    _planes{0, 0, 0, 0, 0, 0} {

}

VROBoundingBox::VROBoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) :
    _planes{xmin, xmax, ymin, ymax, zmin, zmax} {

}

/////////////////////////////////////////////////////////////////////////////////
//
//  Transformation
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Containment and Intersection

void VROBoundingBox::expandBy(float amount) {
    float x = this->getX();
    float y = this->getY();
    float z = this->getZ();
    float xSpan = this->getSpanX();
    float ySpan = this->getSpanY();
    float zSpan = this->getSpanZ();

    _planes[VROBoxPlaneMinX] = x - xSpan / 2 - amount;
    _planes[VROBoxPlaneMaxX] = x + xSpan / 2 + amount;
    _planes[VROBoxPlaneMinY] = y - ySpan / 2 - amount;
    _planes[VROBoxPlaneMaxY] = y + ySpan / 2 + amount;
    _planes[VROBoxPlaneMinZ] = z - zSpan / 2 - amount;
    _planes[VROBoxPlaneMaxZ] = z + zSpan / 2 + amount;
}

void VROBoundingBox::scaleBy(float scale) {
    float x = this->getX();
    float y = this->getY();
    float z = this->getZ();
    float xSpan = this->getSpanX();
    float ySpan = this->getSpanY();
    float zSpan = this->getSpanZ();

    _planes[VROBoxPlaneMinX] = x - (xSpan / 2) * scale;
    _planes[VROBoxPlaneMaxX] = x + (xSpan / 2) * scale;
    _planes[VROBoxPlaneMinY] = y - (ySpan / 2) * scale;
    _planes[VROBoxPlaneMaxY] = y + (ySpan / 2) * scale;
    _planes[VROBoxPlaneMinZ] = z - (zSpan / 2) * scale;
    _planes[VROBoxPlaneMaxZ] = z + (zSpan / 2) * scale;
}

VROBoundingBox VROBoundingBox::transform(VROMatrix4f transform) const {
    const float *t = transform.getArray();
    VROVector3f xa = VROVector3f(t[0], t[1], t[2]) * getMinX();
    VROVector3f xb = VROVector3f(t[0], t[1], t[2]) * getMaxX();

    VROVector3f ya = VROVector3f(t[4], t[5], t[6]) * getMinY();
    VROVector3f yb = VROVector3f(t[4], t[5], t[6]) * getMaxY();

    VROVector3f za = VROVector3f(t[8], t[9], t[10]) * getMinZ();
    VROVector3f zb = VROVector3f(t[8], t[9], t[10]) * getMaxZ();

    VROVector3f xMin(fmin(xa.x, xb.x), fmin(xa.y, xb.y), fmin(xa.z, xb.z));
    VROVector3f yMin(fmin(ya.x, yb.x), fmin(ya.y, yb.y), fmin(ya.z, yb.z));
    VROVector3f zMin(fmin(za.x, zb.x), fmin(za.y, zb.y), fmin(za.z, zb.z));

    VROVector3f xMax(fmax(xa.x, xb.x), fmax(xa.y, xb.y), fmax(xa.z, xb.z));
    VROVector3f yMax(fmax(ya.x, yb.x), fmax(ya.y, yb.y), fmax(ya.z, yb.z));
    VROVector3f zMax(fmax(za.x, zb.x), fmax(za.y, zb.y), fmax(za.z, zb.z));

    VROVector3f min = xMin + yMin + zMin +  transform.extractTranslation();
    VROVector3f max = xMax + yMax + zMax +  transform.extractTranslation();

    return VROBoundingBox(min.x, max.x, min.y, max.y, min.z, max.z);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Containment and Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Containment and Intersection

bool VROBoundingBox::intersectsRay(const VROVector3f &ray, const VROVector3f &origin, VROVector3f *intPt) {
    VROVector3f point;
    VROVector3f normal;
    
    /*
     Check intersections against all planes, return the *closest* intersection.
     */
    bool foundIntersection = false;
    float intersectionDistance = std::numeric_limits<float>::max();
    VROVector3f pt;

    /*
     Check the back plane.
     */
    point.x = _planes[VROBoxPlaneMinX];
    point.y = _planes[VROBoxPlaneMinY];
    point.z = _planes[VROBoxPlaneMinZ];

    normal.x = 0;
    normal.y = 0;
    normal.z = -1;
    
    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    /*
     Check the front plane.
     */
    point.z = _planes[VROBoxPlaneMaxZ];
    normal.z = 1;
    
    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    /*
     Check the left plane.
     */
    normal.x = -1;
    normal.y = 0;
    normal.z = 0;

    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    /*
     Check the bottom plane.
     */
    normal.x = 0;
    normal.y = -1;
    normal.z = 0;

    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    /*
     Check the right plane.
     */
    point.x = _planes[VROBoxPlaneMaxX];
    point.y = _planes[VROBoxPlaneMaxY];

    normal.x = 1;
    normal.y = 0;
    normal.z = 0;

    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    /*
     Check the top plane.
     */
    normal.x = 0;
    normal.y = 1;

    if (ray.rayIntersectPlane(point, normal, origin, &pt)) {
        if (containsPoint(pt)) {
            foundIntersection = true;
            
            float distance = pt.distance(origin);
            if (distance < intersectionDistance) {
                intersectionDistance = distance;
                *intPt = pt;
            }
        }
    }

    return foundIntersection;
}

bool VROBoundingBox::containsPoint(const VROVector3f &point) const {
    if (point.x - _planes[VROBoxPlaneMinX] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.x - _planes[VROBoxPlaneMaxX] > kContainsPointEpsilon) {
        return false;
    }
    if (point.y - _planes[VROBoxPlaneMinY] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.y - _planes[VROBoxPlaneMaxY] > kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMinZ] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMaxZ] > kContainsPointEpsilon) {
        return false;
    }

    return true;
}

bool VROBoundingBox::containsPointXY(const VROVector3f &point) const {
    if (point.x - _planes[VROBoxPlaneMinX] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.x - _planes[VROBoxPlaneMaxX] > kContainsPointEpsilon) {
        return false;
    }
    if (point.y - _planes[VROBoxPlaneMinY] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.y - _planes[VROBoxPlaneMaxY] > kContainsPointEpsilon) {
        return false;
    }

    return true;
}

bool VROBoundingBox::containsPointXZ(const VROVector3f &point) const {
    if (point.x - _planes[VROBoxPlaneMinX] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.x - _planes[VROBoxPlaneMaxX] > kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMinZ] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMaxZ] > kContainsPointEpsilon) {
        return false;
    }

    return true;
}

bool VROBoundingBox::containsPointYZ(const VROVector3f &point) const {
    if (point.y - _planes[VROBoxPlaneMinY] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.y - _planes[VROBoxPlaneMaxY] > kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMinZ] < -kContainsPointEpsilon) {
        return false;
    }
    if (point.z - _planes[VROBoxPlaneMaxZ] > kContainsPointEpsilon) {
        return false;
    }

    return true;
}

bool VROBoundingBox::containsBox(const VROBoundingBox &box) const {
    float x1 = box.getMinX();
    float x2 = box.getMaxX();
    float y1 = box.getMinY();
    float y2 = box.getMaxY();

    return _planes[VROBoxPlaneMinY] <= y1 &&
           _planes[VROBoxPlaneMinX] <= x1 &&
           _planes[VROBoxPlaneMaxY] >= y2 &&
           _planes[VROBoxPlaneMaxX] >= x2;
}

bool VROBoundingBox::intersectsBox(const VROBoundingBox &box) const {
    float distanceX = fabs(box.getX() - getX());
    float distanceY = fabs(box.getY() - getY());

    return distanceX <= (this->getSpanX() + box.getSpanX()) / 2 &&
           distanceY <= (this->getSpanY() + box.getSpanY()) / 2;
}

VROBoundingBox VROBoundingBox::unionWith(const VROBoundingBox &box) {
    float left, right, bottom, top, zmin, zmax = 0;

    if (box.getMinX() < this->getMinX()) {
        left = box.getMinX();
    }
    else {
        left = this->getMinX();
    }

    if (box.getMaxX() > this->getMaxX()) {
        right = box.getMaxX();
    }
    else {
        right = this->getMaxX();
    }

    if (box.getMinY() < this->getMinY()) {
        bottom = box.getMinY();
    }
    else {
        bottom = this->getMinY();
    }

    if (box.getMaxY() > this->getMaxY()) {
        top = box.getMaxY();
    }
    else {
        top = this->getMaxY();
    }

    if (box.getMinZ() < this->getMinZ()) {
        zmin = box.getMinZ();
    }
    else {
        zmin = this->getMinZ();
    }

    if (box.getMaxZ() > this->getMaxZ()) {
        zmax = box.getMaxZ();
    }
    else {
        zmax = this->getMaxZ();
    }

    return VROBoundingBox(left, right, bottom, top, zmin, zmax);
}

void VROBoundingBox::unionDestructive(const VROBoundingBox &box) {
    _planes[VROBoxPlaneMinX] = std::min(_planes[VROBoxPlaneMinX], box._planes[VROBoxPlaneMinX]);
    _planes[VROBoxPlaneMaxX] = std::max(_planes[VROBoxPlaneMaxX], box._planes[VROBoxPlaneMaxX]);
    _planes[VROBoxPlaneMinY] = std::min(_planes[VROBoxPlaneMinY], box._planes[VROBoxPlaneMinY]);
    _planes[VROBoxPlaneMaxY] = std::max(_planes[VROBoxPlaneMaxY], box._planes[VROBoxPlaneMaxY]);
    _planes[VROBoxPlaneMinZ] = std::min(_planes[VROBoxPlaneMinZ], box._planes[VROBoxPlaneMinZ]);
    _planes[VROBoxPlaneMaxZ] = std::max(_planes[VROBoxPlaneMaxZ], box._planes[VROBoxPlaneMaxZ]);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Getters and Setters
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Getters and Setters

void VROBoundingBox::set(const float *dimensions) {
    for (int i = 0; i < 6; i++) {
        _planes[i] = dimensions[i];
    }
}

void VROBoundingBox::set(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
    _planes[VROBoxPlaneMinX] = xMin;
    _planes[VROBoxPlaneMaxX] = xMax;
    _planes[VROBoxPlaneMinY] = yMin;
    _planes[VROBoxPlaneMaxY] = yMax;
    _planes[VROBoxPlaneMinZ] = zMin;
    _planes[VROBoxPlaneMaxZ] = zMax;
}

void VROBoundingBox::copy(const VROBoundingBox &box) {
    _planes[VROBoxPlaneMinX] = box.getMinX();
    _planes[VROBoxPlaneMaxX] = box.getMaxX();
    _planes[VROBoxPlaneMinY] = box.getMinY();
    _planes[VROBoxPlaneMaxY] = box.getMaxY();
    _planes[VROBoxPlaneMinZ] = box.getMinZ();
    _planes[VROBoxPlaneMaxZ] = box.getMaxZ();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Utilities
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Utilities

void VROBoundingBox::center(float *center) const {
    center[0] = (_planes[VROBoxPlaneMinX] + _planes[VROBoxPlaneMaxX]) / 2;
    center[1] = (_planes[VROBoxPlaneMaxY] + _planes[VROBoxPlaneMinY]) / 2;
    center[2] = (_planes[VROBoxPlaneMaxZ] + _planes[VROBoxPlaneMinZ]) / 2;
}

float VROBoundingBox::getDistanceToPoint(VROVector3f p) const {
    float dx = std::max(std::max(getMinX() - p.x, 0.0f), p.x - getMaxX());
    float dy = std::max(std::max(getMinY() - p.y, 0.0f), p.y - getMaxY());
    float dz = std::max(std::max(getMinZ() - p.z, 0.0f), p.z - getMaxZ());

    return sqrt(dx * dx + dy * dy + dz * dz);
}

float VROBoundingBox::getFurthestDistanceToPoint(VROVector3f p) const {
    float dx = std::max(fabs(getMinX() - p.x), fabs(getMaxX() - p.x));
    float dy = std::max(fabs(getMinY() - p.y), fabs(getMaxY() - p.y));
    float dz = std::max(fabs(getMinZ() - p.z), fabs(getMaxZ() - p.z));
    
    return sqrt(dx * dx + dy * dy + dz * dz);
}

std::string VROBoundingBox::toString() const {
    std::stringstream ss;
    ss << std::fixed << "left: " << getMinX() << ", right: " << getMaxX() << ", bottom: " << getMinY() << ", top: " << getMaxY() << ", floor: " << getMinZ() << ", ceiling: " << getMaxZ();

    return ss.str();
}
