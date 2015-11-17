//
//  VRONode.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRONode_h
#define VRONode_h

#include <stdio.h>
#include "VROMatrix4f.h"
#include "VROQuaternion.h"

class VRONode {
    
public:
    
private:
    
    VROVector3f position;
    VROMatrix4f transform;
    VROQuaternion rotation;
    
};

#endif /* VRONode_h */
