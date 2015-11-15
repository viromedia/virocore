//
//  VROVector3d.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROVECTOR3D_H_
#define VROVECTOR3D_H_

#include <stdlib.h>
#include <math.h>
#include <string>

class VROVector3f;

class VROVector3d {
public:
    double x;
    double y;
    double z;

    VROVector3d();
    VROVector3d(double x, double y);
    VROVector3d(double x, double y, double z);
    VROVector3d(VROVector3d const &vector);
    VROVector3d(VROVector3f const &vector);

    /*
     Find the absolute angle between this vector and the given line. The
     direction of the line is not taken into account. The return value will be in
     the range [0, PI/2]. The input line vector must be normalized, as must
     this vector.
     */
    double angleWithLine(const VROVector3d &line) const;

    /*
     Find the angle between this vector and another vector. Both must be
     normalized. Return value in the range [0, PI].
     */
    double angleWithNormedVector(const VROVector3d &vector) const;

    /*
     Return the angle formed between this vector and the given vector about
     the Z axis. Uses the dot product for the magnitude of the angle and the
     cross product for the direction. Return value in the range [-PI, PI].
     */
    double angleZ(const VROVector3d &other) const;

    /*
     Rotate the vector.
     */
    void rotateZ(float angleRad, VROVector3f *result) const;
    void rotateZ(double angleRad, VROVector3d *result) const;
    void rotateAboutAxis(const VROVector3d &axisDir, const VROVector3d &axisPos, double angleRad,
                         VROVector3d *result) const;

    /*
     Intersect the line or ray defined by this vector and the given origin with the plane defined by the given point and normal.
     Store the intersection in the given point. Note that this is only valid if this vector has already been normalized.
     */
    bool lineIntersectPlane(const VROVector3d &point, const VROVector3d &normal, const VROVector3d &origin,
                            VROVector3d *intPt) const;
    bool rayIntersectPlane(const VROVector3d &point, const VROVector3d &normal, const VROVector3d &origin,
                           VROVector3d *intPt) const;

    /*
     Copy operations.
     */
    void set(const VROVector3d &value);
    void set(const VROVector3f &value);
    void set(double x, double y, double z);

    /*
     Basic arithmetic.
     */
    void add(const VROVector3d &vB, VROVector3d *result) const;
    void addScaled(const VROVector3d &scaledB, float scale, VROVector3d *result) const;
    void subtract(const VROVector3d &vB, VROVector3d *result) const;
    void scale(double factor, VROVector3d *result) const;

    /*
     Midpoints and distances.
     */
    void   midpoint(const VROVector3f &other, VROVector3d *result) const;
    void   midpoint(const VROVector3d &other, VROVector3d *result) const;
    void   midpoint(const VROVector3f &other, VROVector3f *result) const;
    void   midpoint(const VROVector3d &other, VROVector3f *result) const;
    double distance(const VROVector3d &vB) const;
    double distanceXY(const VROVector3d &vB) const;
    double distanceSquared(const VROVector3d &vB) const;
    double magnitude() const;
    double magnitudeXY() const;

    double magnitudeXYSquared() const {
        return x * x + y * y;
    }

    /*
     Basic vector operations.
     */
    double dot(const VROVector3d &vB) const;
    void   cross(const VROVector3d &vB, VROVector3d *result) const;
    double normalize();

    /*
     Clearing.
     */
    void clear();
    bool isZero() const;

    /*
     Hashing.
     */
    bool isEqual(const VROVector3d &vector) const;
    int hash() const;

    /*
     Utilities.
     */
    std::string toString() const;

    /*
     Operator overloads.
     */
    inline bool operator ==(const VROVector3d & other) const {
        return other.x == x && other.y == y && other.z == z;
    }

    inline bool operator !=(const VROVector3d & other) const {
        return ! this->operator ==(other);
    }
};

/*
 Operator overloads
 */
inline VROVector3d operator+(VROVector3d lhs, const VROVector3d& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs; // return the result by value
}

inline VROVector3d operator-(VROVector3d lhs, const VROVector3d& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs; // return the result by value
}

inline VROVector3d operator*(VROVector3d lhs, const double multiplier) {
    lhs.x *= multiplier;
    lhs.y *= multiplier;
    lhs.z *= multiplier;
    return lhs; // return the result by value
}

inline VROVector3d operator/(VROVector3d lhs, const float divisor) {
    lhs.x /= divisor;
    lhs.y /= divisor;
    lhs.z /= divisor;
    return lhs; // return the result by value
}

#endif /* VROVECTOR3D_H_ */
