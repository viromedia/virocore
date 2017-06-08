//
//  VROARAnchor.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARAnchor_h
#define VROARAnchor_h

#include "VROMatrix4f.h"

/*
 Real-world position and orientation that can be placed in an AR scene. Once added
 to an AR session, the anchor's position and orientation is continually tracked.
 */
class VROARAnchor {
public:
    
    /*
     Create a new anchor.
     */
    VROARAnchor() {}
    virtual ~VROARAnchor() {}
    
    /*
     Transformation matrix encoding the position, orientation and scale of the
     anchor in world coordinates.
     */
    virtual VROMatrix4f getTransform() const = 0;
    
};

#endif /* VROARAnchor_h */
