//
//  VROLayerSubstrate.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/20/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLayerSubstrate.h"

size_t kCornersInLayer = 6;

void VROLayerSubstrate::buildQuad(VROLayerVertexLayout *vertexLayout) {
    const float x = 1;
    const float y = 1;
    const float z = 1;
    
    vertexLayout[0].x = -x / 2;
    vertexLayout[0].y = -y / 2;
    vertexLayout[0].z = z;
    vertexLayout[0].u = 0;
    vertexLayout[0].v = 0;
    vertexLayout[0].nx = 0;
    vertexLayout[0].ny = 0;
    vertexLayout[0].nz = -1;
    
    vertexLayout[1].x =  x / 2;
    vertexLayout[1].y = -y / 2;
    vertexLayout[1].z = z;
    vertexLayout[1].u = 1;
    vertexLayout[1].v = 0;
    vertexLayout[1].nx = 0;
    vertexLayout[1].ny = 0;
    vertexLayout[1].nz = -1;
    
    vertexLayout[2].x = -x / 2;
    vertexLayout[2].y =  y / 2;
    vertexLayout[2].z = z;
    vertexLayout[2].u = 0;
    vertexLayout[2].v = 1;
    vertexLayout[2].nx = 0;
    vertexLayout[2].ny = 0;
    vertexLayout[2].nz = -1;
    
    vertexLayout[3].x = x / 2;
    vertexLayout[3].y = y / 2;
    vertexLayout[3].z = z;
    vertexLayout[3].u = 1;
    vertexLayout[3].v = 1;
    vertexLayout[3].nx = 0;
    vertexLayout[3].ny = 0;
    vertexLayout[3].nz = -1;
    
    vertexLayout[4].x = -x / 2;
    vertexLayout[4].y =  y / 2;
    vertexLayout[4].z = z;
    vertexLayout[4].u = 0;
    vertexLayout[4].v = 1;
    vertexLayout[4].nx = 0;
    vertexLayout[4].ny = 0;
    vertexLayout[4].nz = -1;
    
    vertexLayout[5].x =  x / 2;
    vertexLayout[5].y = -y / 2;
    vertexLayout[5].z = z;
    vertexLayout[5].u = 1;
    vertexLayout[5].v = 0;
    vertexLayout[5].nx = 0;
    vertexLayout[5].ny = 0;
    vertexLayout[5].nz = -1;
}