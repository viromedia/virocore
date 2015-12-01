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
    
    double &operator[] (const int index) {
        return _mtx[index];
    }
    double const &operator[](int index) const {
        return _mtx[index];
    }

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
    VROMatrix4d multiply(const VROMatrix4d &matrix);
    VROVector3d multiply(const VROVector3d &vector) const;
    
private:
    
    /*
     The data for this matrix.
     */
    double _mtx[16];
    
};

#endif /* VROMatrix4_H_ */
