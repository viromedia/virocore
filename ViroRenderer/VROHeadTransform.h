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
        _headView(matrix_identity_float4x4)
    {}
    
    void setHeadView(matrix_float4x4 headView) {
        _headView = headView;
    }
    matrix_float4x4 getHeadView() const {
        return _headView;
    }
    
    vector_float3 getTranslation() const {
        return (vector_float3) {_headView.columns[3].x, _headView.columns[3].y, _headView.columns[3].z};
    }
    
    vector_float3 getForwardVector() const {
        return (vector_float3) {_headView.columns[2].x, _headView.columns[2].y, _headView.columns[2].z};
    }
    
    vector_float3 getUpVector() const {
        return (vector_float3) {_headView.columns[1].x, _headView.columns[1].y, _headView.columns[1].z};
    }
    
    vector_float3 getRightVector() const {
        return (vector_float3) {_headView.columns[0].x, _headView.columns[0].y, _headView.columns[0].z};
    }
    
    /*
    GLKQuaternion getQuaternion() const {
        return nullptr;
    }
     */
    
    vector_float3 getEulerAngles() const {
        float yaw = 0;
        float roll = 0;
        float pitch = asinf(_headView.columns[1].z);
        if (sqrtf(1.0f - _headView.columns[1].z * _headView.columns[1].z) >= 0.01f) {
            yaw  = atan2f(-_headView.columns[0].z, _headView.columns[3].z);
            roll = atan2f(-_headView.columns[1].x, _headView.columns[1].y);
        }
        else
        {
            yaw = 0.0f;
            roll = atan2f(_headView.columns[0].y, _headView.columns[0].x);
        }
        return (vector_float3) { -pitch, -yaw, -roll };
    }
    
private:
    
    matrix_float4x4 _headView;
    
};

#endif /* VROHeadTransform_h */
