//
//  VROConvert.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROConvert.h"
#include "VROMatrix4f.h"
#include "VROVector4f.h"
#include "VROVector3f.h"
#include "VROCameraTexture.h"

#if VRO_PLATFORM_IOS

VROMatrix4f VROConvert::toMatrix4f(GLKMatrix4 glm) {
    float m[16] = {
        glm.m[0],  glm.m[1],  glm.m[2],  glm.m[3] ,
        glm.m[4],  glm.m[5],  glm.m[6],  glm.m[7] ,
        glm.m[8],  glm.m[9],  glm.m[10], glm.m[11],
        glm.m[12], glm.m[13], glm.m[14], glm.m[15]
    };
    
    return VROMatrix4f(m);
}

VROCameraOrientation VROConvert::toCameraOrientation(UIInterfaceOrientation orientation) {
    if (orientation == UIInterfaceOrientationPortrait) {
        return VROCameraOrientation::Portrait;
    }
    else if (orientation == UIInterfaceOrientationLandscapeLeft) {
        return VROCameraOrientation::LandscapeLeft;
    }
    else if (orientation == UIInterfaceOrientationLandscapeRight) {
        return VROCameraOrientation::LandscapeRight;
    }
    else {
        return VROCameraOrientation::PortraitUpsideDown;
    }
}

UIInterfaceOrientation VROConvert::toDeviceOrientation(VROCameraOrientation orientation) {
    if (orientation == VROCameraOrientation::Portrait) {
        return UIInterfaceOrientationPortrait;
    }
    else if (orientation == VROCameraOrientation::LandscapeLeft) {
        return UIInterfaceOrientationLandscapeLeft;
    }
    else if (orientation == VROCameraOrientation::LandscapeRight) {
        return UIInterfaceOrientationLandscapeRight;
    }
    else {
        return UIInterfaceOrientationPortraitUpsideDown;
    }
}

vector_float3 VROConvert::toVectorFloat3(VROVector3f v) {
    return { v.x, v.y, v.z };
}

vector_float4 VROConvert::toVectorFloat4(VROVector3f v, float w) {
    return { v.x, v.y, v.z, w };
}

vector_float4 VROConvert::toVectorFloat4(VROVector4f v) {
    return { v.x, v.y, v.z, v.w };
}

matrix_float4x4 VROConvert::toMatrixFloat4x4(VROMatrix4f m) {
    matrix_float4x4 m4x4 = {
        .columns[0] = { m[0],  m[1],  m[2],  m[3]  },
        .columns[1] = { m[4],  m[5],  m[6],  m[7]  },
        .columns[2] = { m[8],  m[9],  m[10], m[11] },
        .columns[3] = { m[12], m[13], m[14], m[15] }
    };
    
    return m4x4;
}

VROVector3f VROConvert::toVector3f(vector_float3 v) {
    return { v.x, v.y, v.z };
}

VROMatrix4f VROConvert::toMatrix4f(matrix_float3x3 m) {
    float mtx[16] = { m.columns[0][0], m.columns[0][1], m.columns[0][2], 0,
                      m.columns[1][0], m.columns[1][1], m.columns[1][2], 0,
                      m.columns[2][0], m.columns[2][1], m.columns[2][2], 0,
                      0, 0, 0, 1 };
    return VROMatrix4f(mtx);
}

VROMatrix4f VROConvert::toMatrix4f(matrix_float4x4 m) {
    float mtx[16] = { m.columns[0][0], m.columns[0][1], m.columns[0][2], m.columns[0][3],
                      m.columns[1][0], m.columns[1][1], m.columns[1][2], m.columns[1][3],
                      m.columns[2][0], m.columns[2][1], m.columns[2][2], m.columns[2][3],
                      m.columns[3][0], m.columns[3][1], m.columns[3][2], m.columns[3][3] };
    return VROMatrix4f(mtx);
}

#endif
