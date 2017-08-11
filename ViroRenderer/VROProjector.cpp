//
//  VROProjector.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROProjector.h"
#include "VROMath.h"

bool VROProjector::project(const VROVector3f pos, const float *mvp, const int *viewport, VROVector3f *result) {
    float tin[4];
    float tout[4];
    
    tin[0] = pos.x;
    tin[1] = pos.y;
    tin[2] = pos.z;
    tin[3] = 0;
    
    VROMathMultVectorByMatrix(mvp, tin, tout);
    if (tout[3] == 0.0) {
        return false;
    }
    
    tout[3] = (1.0 / tout[3]) * 0.5;
    
    // Map x, y and z to range 0-1
    tout[0] = tout[0] * tout[3] + 0.5;
    tout[1] = tout[1] * tout[3] + 0.5;
    tout[2] = tout[2] * tout[3] + 0.5;
    
    // Map x,y to viewport
    result->x = viewport[0] + tout[0] * viewport[2];
    result->y = viewport[3] - (viewport[1] + tout[1] * viewport[3]);
    result->z = tout[2];
    
    return true;
}

bool VROProjector::unproject(const VROVector3f screen, const float *mvp, const int *viewport, VROVector3f *result) {
    float mvpInverse[16];
    if (!VROMathInvertMatrix(mvp, mvpInverse)) {
        return false;
    }
    
    float tempIn[4];
    float tempOut[4];
    
    tempIn[0] = screen.x;
    tempIn[1] = screen.y;
    tempIn[2] = screen.z;
    tempIn[3] = 1.0;
    
    // Map x and y from window coordinates. Y coordinate gets flipped here.
    tempIn[0] = (tempIn[0] - viewport[0]) / viewport[2];
    tempIn[1] = (tempIn[1] - viewport[3] + viewport[1]) / -viewport[3];
    
    // Map to range -1 to 1
    tempIn[0] = tempIn[0] * 2 - 1;
    tempIn[1] = tempIn[1] * 2 - 1;
    tempIn[2] = tempIn[2] * 2 - 1;
    
    VROMathMultVectorByMatrix(mvpInverse, tempIn, tempOut);
    if (tempOut[3] == 0.0) {
        return false;
    }
    
    tempOut[3] = 1.0 / tempOut[3];
    
    result->x = tempOut[0] * tempOut[3];
    result->y = tempOut[1] * tempOut[3];
    result->z = tempOut[2] * tempOut[3];
    
    return true;
}
