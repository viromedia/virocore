//
//  VROConvert.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROConvert.h"
#include "VROMatrix4f.h"

VROMatrix4f VROConvert::toMatrix4f(GLKMatrix4 glm) {
    float m[16] = {
        glm.m[0],  glm.m[1],  glm.m[2],  glm.m[3] ,
        glm.m[4],  glm.m[5],  glm.m[6],  glm.m[7] ,
        glm.m[8],  glm.m[9],  glm.m[10], glm.m[11],
        glm.m[12], glm.m[13], glm.m[14], glm.m[15]
    };
    
    return VROMatrix4f(m);
}
