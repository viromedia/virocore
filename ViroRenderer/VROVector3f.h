//
//  VROVector3h.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROVECTOR3F_H_
#define VROVECTOR3F_H_

#include <stdlib.h>
#include <math.h>
#include <string>

class VROVector3d;

class VROVector3f {
public:
    float x;
    float y;
    float z;

    VROVector3f();
    VROVector3f(float x, float y);
    VROVector3f(float x, float y, float z);
    VROVector3f(const VROVector3f &vector);
    virtual ~VROVector3f();
    
    VROVector3f &operator*=(const float multiplier) {
        x *= multiplier;
        y *= multiplier;
        z *= multiplier;
        return *this;
    }
    
    VROVector3f &operator/=(const float divisor) {
        x /= divisor;
        y /= divisor;
        z /= divisor;
        return *this;
    }
    
    VROVector3f &operator+=(const VROVector3f &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    /*
     Find the absolute angle between this vector and the given line. The
     direction of the line is not taken into account. The return value will be in
     the range [0, PI/2]. The input line vector must be normalized, as must
     this vector.
     */
    float angleWithLine(const VROVector3f &line) const;

    /*
     Find the angle between this vector and another vector. Both must be
     normalized. Return value in the range [0, PI].
     */
    float angleWithNormedVector(const VROVector3f &vector) const;

    /*
     Return the angle formed between this vector and the given vector about the Z axis.
     This method assumes away each vector's Z coordinate, treating them as if they're 0.

     Uses the dot product for the magnitude of the angle and the cross product for the
     direction. Positive angles are counter-clockwise, negative angles are clockwise.
     Return value in the range [-PI, PI].
     */
    double angleZ(const VROVector3f &other) const;

    /*
     Optimized angle Z computation for use when both this vector and the given vector have
     no Z coordinate and are already normalized. Positive angles are counter-clockwise, negative angles
     are clockwise. Return value is in the range [-PI, PI].
     */
    float angleZ_normed(const VROVector3f &other) const;

    /*
     Optimized angle Z computation, returns the angle this vector makes against the X axis.
     Positive angles are counter-clockwise, negative angles are clockwise. Return value is
     in the range (-PI, PI]. This vector does not need to be normalized.

     Implementation note: this just wraps atan2 using the x and y of this vector.
     */
    float angleZ_xAxis() const;

    /*
     Rotate the vector.
     */
    void rotateZ(float angleRad, VROVector3f *result) const;
    void rotateAboutAxis(const VROVector3f &axisDir, const VROVector3f &axisPos, float angleRad, VROVector3f *result) const;

    /*
     Intersect the line or ray defined by this vector and the given origin with the plane defined by the given point and normal.
     Store the intersection in the given point. Note that this is only valid if this vector has already been normalized.
     */
    bool lineIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                            const VROVector3f &origin, VROVector3f *intPt) const;
    bool lineIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                            const VROVector3d &origin, VROVector3d *intPt) const;
    bool rayIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                           const VROVector3f &origin, VROVector3f *intPt) const;
    bool rayIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                           const VROVector3d &origin, VROVector3d *intPt) const;

    /*
     Copy operations.
     */
    void set(const VROVector3f &value);
    void set(const VROVector3d &value);
    void set(float x, float y, float z);

    /*
     Basic arithmetic.
     */
    VROVector3f add(VROVector3f vB) const;
    void add(const VROVector3f &vB, VROVector3f *result) const;
    void addScaled(const VROVector3f &scaledB, float scale, VROVector3f *result) const;
    VROVector3f subtract(VROVector3f vB) const;
    void subtract(const VROVector3f &vB, VROVector3f *result) const;
    void scale(float factor, VROVector3f *result) const;

    /*
     Midpoints and distances.
     */
    void   midpoint(const VROVector3f &other, VROVector3f *result) const;
    void   midpoint(const VROVector3d &other, VROVector3f *result) const;
    float  distance(const VROVector3f &vB) const;
    float  distanceAccurate(const VROVector3f &vB) const;
    float  distance(const VROVector3d &vB) const;
    float  distanceXY(const VROVector3f &vB) const;
    float  distanceSquared(const VROVector3f &vB) const;
    float  magnitude() const;
    float  magnitudeXY() const;

    /*
     Basic vector operations.
     */
    float  dot(const VROVector3f &vB) const;
    double dot(const VROVector3d &vB) const;
    void   cross(const VROVector3f &vB, VROVector3f *result) const;
    VROVector3f cross(const VROVector3f &vB) const;
    VROVector3f normalize() const;
    VROVector3f interpolate(VROVector3f other, float t);

    /*
     Clearing.
     */
    void   clear();
    bool   isZero() const;

    /*
     Hashing.
     */
    bool isEqual(const VROVector3f &vector) const;
    int hash() const;

    /*
     Utilities.
     */
    std::string toString() const;
};

/*
 Operator overloads.
 */
inline VROVector3f operator+(VROVector3f lhs, const VROVector3f& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs; // return the result by value
}

inline VROVector3f operator-(VROVector3f lhs, const VROVector3f& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs; // return the result by value
}

inline VROVector3f operator*(VROVector3f lhs, const float multiplier) {
    lhs.x *= multiplier;
    lhs.y *= multiplier;
    lhs.z *= multiplier;
    return lhs; // return the result by value
}

inline VROVector3f operator/(VROVector3f lhs, const float divisor) {
    lhs.x /= divisor;
    lhs.y /= divisor;
    lhs.z /= divisor;
    return lhs; // return the result by value
}

#endif /* VROVECTOR3F_H_ */
