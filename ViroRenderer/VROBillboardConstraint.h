//
//  VROBillboardConstraint.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROBillboardConstraint_h
#define VROBillboardConstraint_h

#include "VROConstraint.h"

class VROBillboardConstraint : public VROConstraint {
    
public:
    
    virtual VROMatrix4f getTransform(const VRONode &node, VROMatrix4f transform);
    
    
private:
    
};

#endif /* VROBillboardConstraint_h */
