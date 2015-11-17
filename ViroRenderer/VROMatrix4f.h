//
//  VROMatrix4f.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMATRIX_H_
#define VROMATRIX_H_

#include <stdlib.h>
#include <math.h>

class VROVector3f;

class VROMatrix4f {
public:
    
    float &operator[] (const int index) {
        return mtx[index];
    }
    float const &operator[](int index) const {
        return mtx[index];
    }

    VROMatrix4f();
    VROMatrix4f(const float *matrix);
    VROMatrix4f(const float *matrix, bool owner);
    virtual ~VROMatrix4f();

    void toIdentity();
    void copy(const VROMatrix4f &copy);

    /*
     Scale.
     */
    void scale(float x, float y, float z);

    /*
     Rotation.
     */
    void rotateX(float angleRad);
    void rotateY(float angleRad);
    void rotateZ(float angleRad);
    void rotate(float angleRad, const VROVector3f &origin, const VROVector3f &dir);
    void setRotationCenter(const VROVector3f &center, const VROVector3f &translation);

    /*
     Translation.
     */
    void translate(float x, float y, float z);
    void translate(const VROVector3f &vector);

    /*
     Multiplication.
     */
    void preMultiply(const VROMatrix4f &A) ;
    void postMultiply(const VROMatrix4f &B) ;
    void multiplyVector(const VROVector3f &vector, VROVector3f *result) const ;
    
private:
    
    /*
     The 16-float data for this matrix.
     */
    float mtx[16];
    
};

#endif /* VROMATRIX_H_ */
