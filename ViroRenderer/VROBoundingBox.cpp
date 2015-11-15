//
//  VROBoundingBox.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROBoundingBox.h"
#include <sstream>
#include "VROVector3d.h"
#include "VROVector3f.h"

//Epsilon value for point containment to account for precision errors
#define kContainsPointEpsilon 0.5

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
    _planes{0, 0, 0, 0, 0, 0},
    _sourceFrustumForDistances(nullptr),
    _distanceFrame(UINT32_MAX),
    _planeLastOutside(0),
    _distancesValid(false) {
        
}

VROBoundingBox::VROBoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) :
    _planes{xmin, xmax, ymin, ymax, zmin, zmax},
    _sourceFrustumForDistances(nullptr),
    _distanceFrame(UINT32_MAX),
    _planeLastOutside(0),
    _distancesValid(false) {

}

VROBoundingBox::~VROBoundingBox() {

}

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

/////////////////////////////////////////////////////////////////////////////////
//
//  Containment and Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Containment and Intersection

bool VROBoundingBox::intersectsRay(const VROVector3d &ray, const VROVector3d &origin, VROVector3d *intPt) {
    VROVector3d point;
    VROVector3d normal;

    /*
     Check the floor plane.
     */
    point.x = _planes[VROBoxPlaneMinX];
    point.y = _planes[VROBoxPlaneMinY];
    point.z = _planes[VROBoxPlaneMinZ];

    normal.x = 0;
    normal.y = 0;
    normal.z = 1;

    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    /*
     Check the ceiling plane.
     */
    point.z = _planes[VROBoxPlaneMaxZ];
    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    /*
     Check the left plane.
     */
    normal.x = -1;
    normal.y = 0;
    normal.z = 0;

    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    /*
     Check the bottom plane.
     */
    normal.x = 0;
    normal.y = -1;
    normal.z = 0;

    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
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

    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    /*
     Check the top plane.
     */
    normal.x = 0;
    normal.y = 1;

    if (ray.rayIntersectPlane(point, normal, origin, intPt)) {
        if (containsPoint(*intPt)) {
            return true;
        }
    }

    return false;
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

bool VROBoundingBox::containsPoint(const VROVector3d &point) const {
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

VROBoundingBox* VROBoundingBox::unionWith(const VROBoundingBox &box) {
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

    return new VROBoundingBox(left, right, bottom, top, zmin, zmax);
}

void VROBoundingBox::unionLRBTDestructive(const VROBoundingBox &box) {
    _planes[VROBoxPlaneMinX] = std::min(_planes[VROBoxPlaneMinX], box._planes[VROBoxPlaneMinX]);
    _planes[VROBoxPlaneMaxX] = std::max(_planes[VROBoxPlaneMaxX], box._planes[VROBoxPlaneMaxX]);
    _planes[VROBoxPlaneMinY] = std::min(_planes[VROBoxPlaneMinY], box._planes[VROBoxPlaneMinY]);
    _planes[VROBoxPlaneMaxY] = std::max(_planes[VROBoxPlaneMaxY], box._planes[VROBoxPlaneMaxY]);
    _planes[VROBoxPlaneMinZ] = std::min(_planes[VROBoxPlaneMinZ], box._planes[VROBoxPlaneMinZ]);
    _planes[VROBoxPlaneMaxZ] = std::max(_planes[VROBoxPlaneMaxZ], box._planes[VROBoxPlaneMaxZ]);

    resetFrustumDistances();
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

    resetFrustumDistances();
}

void VROBoundingBox::set(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
    _planes[VROBoxPlaneMinX] = xMin;
    _planes[VROBoxPlaneMaxX] = xMax;
    _planes[VROBoxPlaneMinY] = yMin;
    _planes[VROBoxPlaneMaxY] = yMax;
    _planes[VROBoxPlaneMinZ] = zMin;
    _planes[VROBoxPlaneMaxZ] = zMax;
    
    resetFrustumDistances();
}

void VROBoundingBox::copy(const VROBoundingBox &box) {
    _planes[VROBoxPlaneMinX] = box.getMinX();
    _planes[VROBoxPlaneMaxX] = box.getMaxX();
    _planes[VROBoxPlaneMinY] = box.getMinY();
    _planes[VROBoxPlaneMaxY] = box.getMaxY();
    _planes[VROBoxPlaneMinZ] = box.getMinZ();
    _planes[VROBoxPlaneMaxZ] = box.getMaxZ();

    resetFrustumDistances();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Utilities
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Utilities

void VROBoundingBox::resetFrustumDistances() {
    _distanceFrame = UINT32_MAX;
    _sourceFrustumForDistances = nullptr;
    _distancesValid = false;
}

void VROBoundingBox::center(float *center) const {
    center[0] = (_planes[VROBoxPlaneMinX] + _planes[VROBoxPlaneMaxX]) / 2;
    center[1] = (_planes[VROBoxPlaneMaxY] + _planes[VROBoxPlaneMinY]) / 2;
    center[2] = (_planes[VROBoxPlaneMaxZ] + _planes[VROBoxPlaneMinZ]) / 2;
}

std::string VROBoundingBox::toString() const {
    std::stringstream ss;
    ss << std::fixed << "left: " << getMinX() << ", right: " << getMaxX() << ", bottom: " << getMinY() << ", top: " << getMaxY() << ", floor: " << getMinZ() << ", ceiling: " << getMaxZ();

    return ss.str();
}
