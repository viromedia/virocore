//
//  VROHeadTransform.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROHeadTransform_h
#define VROHeadTransform_h

#include <stdio.h>
#include <simd/simd.h>

class VROHeadTransform {
  
public:
    
    VROHeadTransform() :
        _headRotation(matrix_identity_float4x4)
    {}
    
    void setHeadRotation(matrix_float4x4 headRotation) {
        _headRotation = headRotation;
    }
    matrix_float4x4 getHeadRotation() const {
        return _headRotation;
    }
    
    vector_float3 getForwardVector() const {
        return (vector_float3) {_headRotation.columns[2].x, _headRotation.columns[2].y, _headRotation.columns[2].z};
    }
    
    vector_float3 getUpVector() const {
        return (vector_float3) {_headRotation.columns[1].x, _headRotation.columns[1].y, _headRotation.columns[1].z};
    }
    
    vector_float3 getRightVector() const {
        return (vector_float3) {_headRotation.columns[0].x, _headRotation.columns[0].y, _headRotation.columns[0].z};
    }
    
    vector_float3 getEulerAngles() const {
        float yaw = 0;
        float roll = 0;
        float pitch = asinf(_headRotation.columns[1].z);
        if (sqrtf(1.0f - _headRotation.columns[1].z * _headRotation.columns[1].z) >= 0.01f) {
            yaw  = atan2f(-_headRotation.columns[0].z, _headRotation.columns[3].z);
            roll = atan2f(-_headRotation.columns[1].x, _headRotation.columns[1].y);
        }
        else
        {
            yaw = 0.0f;
            roll = atan2f(_headRotation.columns[0].y, _headRotation.columns[0].x);
        }
        return (vector_float3) { -pitch, -yaw, -roll };
    }
    
private:
    
    matrix_float4x4 _headRotation;
    
};

#endif /* VROHeadTransform_h */
