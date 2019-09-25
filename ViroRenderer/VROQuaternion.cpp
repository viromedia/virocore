//
//  VROQuaternion.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/8/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#include "VROQuaternion.h"
#include "VROMath.h"

// Constructor which converts euler angles to a quaternion
VROQuaternion::VROQuaternion(float x, float y, float z) {
    set(x,y,z);
}

// Constructor which converts euler angles to a quaternion
VROQuaternion::VROQuaternion(const VROVector3f& vec) {
    set(vec.x, vec.y, vec.z);
}

// Constructor which converts a matrix to a quaternion
VROQuaternion::VROQuaternion(const VROMatrix4f& mat) {
    (*this) = mat;
}

// equal operator
bool VROQuaternion::operator==(const VROQuaternion &other) const {
    return ((X == other.X) &&
            (Y == other.Y) &&
            (Z == other.Z) &&
            (W == other.W));
}

// inequality operator
bool VROQuaternion::operator!=(const VROQuaternion &other) const {
    return !(*this == other);
}

// matrix assignment operator
VROQuaternion &VROQuaternion::operator=(const VROMatrix4f& m) {
    const float diag = m[0] + m[5] + m[10] + 1;
    
    if( diag > 0.0f ) {
        const float scale = sqrtf(diag) * 2.0f; // get scale from diagonal
        
        // TODO: speed this up
        X = (m[6] - m[9]) / scale;
        Y = (m[8] - m[2]) / scale;
        Z = (m[1] - m[4]) / scale;
        W = 0.25f * scale;
    }
    else {
        if (m[0]>m[5] && m[0]>m[10]) {
            // 1st element of diag is greatest value
            // find scale according to 1st element, and double it
            const float scale = sqrtf(1.0f + m[0] - m[5] - m[10]) * 2.0f;
            
            // TODO: speed this up
            X = 0.25f * scale;
            Y = (m[4] + m[1]) / scale;
            Z = (m[2] + m[8]) / scale;
            W = (m[6] - m[9]) / scale;
        }
        else if (m[5]>m[10]) {
            // 2nd element of diag is greatest value
            // find scale according to 2nd element, and double it
            const float scale = sqrtf(1.0f + m[5] - m[0] - m[10]) * 2.0f;
            
            // TODO: speed this up
            X = (m[4] + m[1]) / scale;
            Y = 0.25f * scale;
            Z = (m[9] + m[6]) / scale;
            W = (m[8] - m[2]) / scale;
        }
        else {
            // 3rd element of diag is greatest value
            // find scale according to 3rd element, and double it
            const float scale = sqrtf(1.0f + m[10] - m[0] - m[5]) * 2.0f;
            
            // TODO: speed this up
            X = (m[8] + m[2]) / scale;
            Y = (m[9] + m[6]) / scale;
            Z = 0.25f * scale;
            W = (m[1] - m[4]) / scale;
        }
    }
    
    return normalize();
}

// multiplication operator
VROQuaternion VROQuaternion::operator*(const VROQuaternion &other) const {
    VROQuaternion tmp;
    
    tmp.W = (other.W * W) - (other.X * X) - (other.Y * Y) - (other.Z * Z);
    tmp.X = (other.W * X) + (other.X * W) + (other.Y * Z) - (other.Z * Y);
    tmp.Y = (other.W * Y) + (other.Y * W) + (other.Z * X) - (other.X * Z);
    tmp.Z = (other.W * Z) + (other.Z * W) + (other.X * Y) - (other.Y * X);
    
    return tmp;
}

// multiplication operator
VROQuaternion VROQuaternion::operator*(float s) const {
    return VROQuaternion(s*X, s*Y, s*Z, s*W);
}


// multiplication operator
VROQuaternion &VROQuaternion::operator*=(float s) {
    X*=s;
    Y*=s;
    Z*=s;
    W*=s;
    return *this;
}

// multiplication operator
VROQuaternion &VROQuaternion::operator*=(const VROQuaternion &other) {
    return (*this = other * (*this));
}

// add operator
VROQuaternion VROQuaternion::operator+(const VROQuaternion &b) const {
    return VROQuaternion(X + b.X, Y + b.Y, Z + b.Z, W + b.W);
}

VROQuaternion VROQuaternion::operator-(const VROQuaternion &b) const {
    return VROQuaternion(X - b.X, Y - b.Y, Z - b.Z, W - b.W );
}

// Creates a matrix from this quaternion
VROMatrix4f VROQuaternion::getMatrix() const {
    VROMatrix4f m;
    getMatrix(m);
    return m;
}

/*!
	Creates a matrix from this quaternion
 */
void VROQuaternion::getMatrix(VROMatrix4f &dest,
                              const VROVector3f &center) const {
    dest[0] = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z;
    dest[1] = 2.0f*X*Y + 2.0f*Z*W;
    dest[2] = 2.0f*X*Z - 2.0f*Y*W;
    dest[3] = 0.0f;
    
    dest[4] = 2.0f*X*Y - 2.0f*Z*W;
    dest[5] = 1.0f - 2.0f*X*X - 2.0f*Z*Z;
    dest[6] = 2.0f*Z*Y + 2.0f*X*W;
    dest[7] = 0.0f;
    
    dest[8] = 2.0f*X*Z + 2.0f*Y*W;
    dest[9] = 2.0f*Z*Y - 2.0f*X*W;
    dest[10] = 1.0f - 2.0f*X*X - 2.0f*Y*Y;
    dest[11] = 0.0f;
    
    dest[12] = center.x;
    dest[13] = center.y;
    dest[14] = center.z;
    dest[15] = 1.f;
}


/*!
	Creates a matrix from this quaternion
	Rotate about a center point
	shortcut for
	quaternion q;
	q.rotationFromTo(vin[i].Normal, forward);
	q.getMatrix(lookat, center);
 
	VROMatrix4f m2;
	m2.setInverseTranslation(center);
	lookat *= m2;
 */
void VROQuaternion::getMatrixCenter(VROMatrix4f &dest,
                                           const VROVector3f &center,
                                           const VROVector3f &translation) const {
    dest[0] = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z;
    dest[1] = 2.0f*X*Y + 2.0f*Z*W;
    dest[2] = 2.0f*X*Z - 2.0f*Y*W;
    dest[3] = 0.0f;
    
    dest[4] = 2.0f*X*Y - 2.0f*Z*W;
    dest[5] = 1.0f - 2.0f*X*X - 2.0f*Z*Z;
    dest[6] = 2.0f*Z*Y + 2.0f*X*W;
    dest[7] = 0.0f;
    
    dest[8] = 2.0f*X*Z + 2.0f*Y*W;
    dest[9] = 2.0f*Z*Y - 2.0f*X*W;
    dest[10] = 1.0f - 2.0f*X*X - 2.0f*Y*Y;
    dest[11] = 0.0f;
    
    dest.setRotationCenter ( center, translation );
}

// Creates a matrix from this quaternion
void VROQuaternion::getMatrix_transposed(VROMatrix4f &dest) const {
    dest[0] = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z;
    dest[4] = 2.0f*X*Y + 2.0f*Z*W;
    dest[8] = 2.0f*X*Z - 2.0f*Y*W;
    dest[12] = 0.0f;
    
    dest[1] = 2.0f*X*Y - 2.0f*Z*W;
    dest[5] = 1.0f - 2.0f*X*X - 2.0f*Z*Z;
    dest[9] = 2.0f*Z*Y + 2.0f*X*W;
    dest[13] = 0.0f;
    
    dest[2] = 2.0f*X*Z + 2.0f*Y*W;
    dest[6] = 2.0f*Z*Y - 2.0f*X*W;
    dest[10] = 1.0f - 2.0f*X*X - 2.0f*Y*Y;
    dest[14] = 0.0f;
    
    dest[3] = 0.f;
    dest[7] = 0.f;
    dest[11] = 0.f;
    dest[15] = 1.f;
}


// Inverts this quaternion
VROQuaternion &VROQuaternion::makeInverse()
{
    X = -X; Y = -Y; Z = -Z;
    return *this;
}


// sets new quaternion
VROQuaternion &VROQuaternion::set(float x, float y, float z, float w) {
    X = x;
    Y = y;
    Z = z;
    W = w;
    return *this;
}


// sets new quaternion based on euler angles
VROQuaternion &VROQuaternion::set(float x, float y, float z) {
    double angle;
    
    angle = x * 0.5;
    const double sr = sin(angle);
    const double cr = cos(angle);
    
    angle = y * 0.5;
    const double sp = sin(angle);
    const double cp = cos(angle);
    
    angle = z * 0.5;
    const double sy = sin(angle);
    const double cy = cos(angle);
    
    const double cpcy = cp * cy;
    const double spcy = sp * cy;
    const double cpsy = cp * sy;
    const double spsy = sp * sy;
    
    X = (float)(sr * cpcy - cr * spsy);
    Y = (float)(cr * spcy + sr * cpsy);
    Z = (float)(cr * cpsy - sr * spcy);
    W = (float)(cr * cpcy + sr * spsy);
    
    return normalize();
}

// sets new quaternion based on euler angles
VROQuaternion &VROQuaternion::set(const VROVector3f& vec) {
    return set(vec.x, vec.y, vec.z);
}

// sets new quaternion based on other quaternion
VROQuaternion &VROQuaternion::set(const VROQuaternion &quat) {
    return (*this=quat);
}


//! returns if this quaternion equals the other one, taking floating point rounding errors into account
bool VROQuaternion::equals(const VROQuaternion &other, const float tolerance) const {
    return VROMathEquals(X, other.X, tolerance) &&
    VROMathEquals(Y, other.Y, tolerance) &&
    VROMathEquals(Z, other.Z, tolerance) &&
    VROMathEquals(W, other.W, tolerance);
}


// normalizes the quaternion
VROQuaternion &VROQuaternion::normalize() {
    const float n = X*X + Y*Y + Z*Z + W*W;
    
    if (n == 1)
        return *this;
    
    //n = 1.0f / sqrtf(n);
    return (*this *= VROMathReciprocalSquareRoot(n));
}

float VROQuaternion::getNorm() {
    return sqrt(dotProduct(*this));
}

// set this quaternion to the result of the linear interpolation between two quaternions
VROQuaternion VROQuaternion::lerp(VROQuaternion q1, VROQuaternion q2, float time) {
    const float scale = 1.0f - time;
    return (q1 * scale) + (q2 * time);
}

// set this quaternion to the result of the interpolation between two quaternions
VROQuaternion VROQuaternion::slerp(VROQuaternion q1, VROQuaternion q2, float time, float threshold) {
    float angle = q1.dotProduct(q2);
    
    // make sure we use the short rotation
    if (angle < 0.0f) {
        q1 *= -1.0f;
        angle *= -1.0f;
    }
    
    if (angle <= (1-threshold)) {
        const float theta = acosf(angle);
        const float invsintheta = VROMathReciprocal(sinf(theta));
        const float scale = sinf(theta * (1.0f-time)) * invsintheta;
        const float invscale = sinf(theta * time) * invsintheta;
        
        return (q1 * scale) + (q2 * invscale);
    }
    else // linear interpolation
        return lerp(q1, q2, time);
}


// calculates the dot product
float VROQuaternion::dotProduct(const VROQuaternion &q2) const {
    return (X * q2.X) + (Y * q2.Y) + (Z * q2.Z) + (W * q2.W);
}


//! axis must be unit length, angle in radians
VROQuaternion VROQuaternion::fromAngleAxis(float angle, const VROVector3f& axis) {
    const float fHalfAngle = 0.5f*angle;
    const float fSin = sinf(fHalfAngle);
    
    VROQuaternion q;
    q.W = cosf(fHalfAngle);
    q.X = fSin * axis.x;
    q.Y = fSin * axis.y;
    q.Z = fSin * axis.z;
    
    return q;
}


void VROQuaternion::toAngleAxis(float &angle, VROVector3f &axis) const {
    const float scale = sqrtf(X*X + Y*Y + Z*Z);
    
    if (VROMathIsZero(scale) || W > 1.0f || W < -1.0f) {
        angle = 0.0f;
        axis.x = 0.0f;
        axis.y = 1.0f;
        axis.z = 0.0f;
    }
    else {
        const float invscale = VROMathReciprocal(scale);
        angle = 2.0f * acosf(W);
        axis.x = X * invscale;
        axis.y = Y * invscale;
        axis.z = Z * invscale;
    }
}

float VROQuaternion::getAngle() const {
    const float scale = sqrtf(X*X + Y*Y + Z*Z);
    if (VROMathIsZero(scale) || W > 1.0f || W < -1.0f) {
        return 0;
    }
    else {
        return 2.0f * acosf(W);
    }
}

VROVector3f VROQuaternion::toEuler() const {
    VROVector3f euler;
    const double sqw = W*W;
    const double sqx = X*X;
    const double sqy = Y*Y;
    const double sqz = Z*Z;
    const double test = 2.0 * (Y*W - X*Z);
    
    if (VROMathEquals(test, 1.0, 0.000001)) {
        // heading = rotation about z-axis
        euler.z = (float) (-2.0*atan2(X, W));
        // bank = rotation about x-axis
        euler.x = 0;
        // attitude = rotation about y-axis
        euler.y = (float) (M_PI / 2.0);
    }
    else if (VROMathEquals(test, -1.0, 0.000001)) {
        // heading = rotation about z-axis
        euler.z = (float) (2.0*atan2(X, W));
        // bank = rotation about x-axis
        euler.x = 0;
        // attitude = rotation about y-axis
        euler.y = (float) (M_PI / -2.0);
    }
    else
    {
        // heading = rotation about z-axis
        euler.z = (float) atan2(2.0 * (X*Y +Z*W),(sqx - sqy - sqz + sqw));
        // bank = rotation about x-axis
        euler.x = (float) atan2(2.0 * (Y*Z +X*W),(-sqx - sqy + sqz + sqw));
        // attitude = rotation about y-axis
        euler.y = (float) asin( clamp(test, -1.0, 1.0) );
    }
    
    return euler;
}

VROVector3f VROQuaternion::operator* (const VROVector3f& v) const {
    // nVidia SDK implementation
    
    VROVector3f uv, uuv;
    VROVector3f qvec(X, Y, Z);
    uv = qvec.cross(v);
    uuv = qvec.cross(uv);
    uv *= (2.0f * W);
    uuv *= 2.0f;
    
    return v + uv + uuv;
}

// set quaternion to identity
VROQuaternion &VROQuaternion::makeIdentity() {
    W = 1.f;
    X = 0.f;
    Y = 0.f;
    Z = 0.f;
    return *this;
}

VROQuaternion VROQuaternion::rotationFromTo(const VROVector3f& from, const VROVector3f& to) {
    // Based on Stan Melax's article in Game Programming Gems
    // Copy, since cannot modify local
    VROVector3f v0 = from.normalize();
    VROVector3f v1 = to.normalize();
    
    const float d = v0.dot(v1);
    if (d >= 1.0f) // If dot == 1, vectors are the same
    {
        VROQuaternion q;
        return q.makeIdentity();
    }
    else if (d <= -1.0f) // exactly opposite
    {
        VROVector3f axis(1.0f, 0.f, 0.f);
        axis = axis.cross(v0);
        if (axis.magnitude() == 0) {
            axis.set(0.f,1.f,0.f);
            axis = axis.cross(v0);
        }
        // same as fromAngleAxis(PI, axis).normalize();
        
        VROQuaternion q(axis.x, axis.y, axis.z, 0);
        return q.normalize();
    }
    
    const float s = sqrtf( (1+d)*2 ); // optimize inv_sqrt
    const float invs = 1.f / s;
    const VROVector3f c = v0.cross(v1)*invs;
    
    VROQuaternion q(c.x, c.y, c.z, s * 0.5f);
    return q.normalize();
}
