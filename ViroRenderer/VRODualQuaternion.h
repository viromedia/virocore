//
//  VRODualQuaternion.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/23/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODualQuaternion_h
#define VRODualQuaternion_h

#include "VROQuaternion.h"
#include "VROVector3f.h"

class VRODualQuaternion {
    
public:
    
    VRODualQuaternion();
    VRODualQuaternion(VROQuaternion real, VROQuaternion dual);
    VRODualQuaternion(VROMatrix4f matrix);
    VRODualQuaternion(VROVector3f translation, VROQuaternion rotation, VROVector3f scale);
    
    VRODualQuaternion operator* (const VRODualQuaternion& right) const;
    VRODualQuaternion operator* (float c) const;
    
    VROQuaternion getReal() { return _real; }
    VROQuaternion getDual() { return _dual; }
    
    void normalize();
    
private:
    
    VROQuaternion _real;
    VROQuaternion _dual;
    
};

#endif /* VRODualQuaternion_h */

