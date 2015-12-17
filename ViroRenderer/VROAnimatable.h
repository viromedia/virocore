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
    
    static void animate(std::shared_ptr<VROAnimatable> animatable,
                        std::shared_ptr<VROAnimation> animation);
    
};

#endif /* VROAnimatable_hpp */
