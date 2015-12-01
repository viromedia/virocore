//
//  VROMetalUtils.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMetalUtils.h"
#include "VROVector4f.h"

vector_float4 toVectorFloat4(VROVector4f v) {
    return { v.x, v.y, v.z, v.w };
}
