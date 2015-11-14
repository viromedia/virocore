//
//  VROMatrix4d.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMatrix4_H_
#define VROMatrix4_H_

#include <stdlib.h>
#include <math.h>

class VROVector3d;

class VROMatrix4d {
public:

    /*
     The data for this matrix.
     */
    double mtx[16];

    VROMatrix4d();
    VROMatrix4d(const VROMatrix4d &toCopy);
    VROMatrix4d(const double *matrix);
    virtual ~VROMatrix4d();

    void toIdentity();
    void copy(const VROMatrix4d &copy);

    /*
     Scale.
     */
    void scale(double x, double y, double z);

    /*
     Rotation.
     */
    void rotateX(double angleRad);
    void rotateY(double angleRad);
    void rotateZ(double angleRad);
    void rotate(double angleRad, const VROVector3d &origin, const VROVector3d &dir);

    /*
     Translation.
     */
    void translate(double x, double y, double z);
    void translate(const VROVector3d &vector);

    /*
     Multiplication.
     */
    void preMultiply(const VROMatrix4d &A);
    void postMultiply(const VROMatrix4d &B);
    void multiplyVector(const VROVector3d &vector, VROVector3d *result) const;
};

#endif /* VROMatrix4_H_ */
