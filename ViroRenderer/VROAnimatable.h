//
//  VROAnimatable.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROAnimatable_h
#define VROAnimatable_h

#include <stdio.h>
#include <string>
#include "VROVector3f.h"

class VROAnimation;

/*
 Marker class objects that have animatable properties.
 */
class VROAnimatable {
public:
    
    virtual void setProperty(std::string property, float value) {}
    virtual void setProperty(std::string property, VROVector3f value) {}
    
};

#endif /* VROAnimatable_hpp */
