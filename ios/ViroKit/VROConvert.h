//
//  VROConvert.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROConvert_h
#define VROConvert_h

#include <stdio.h>
#include <GLKit/GLKit.h>

class VROMatrix4f;

/*
 Utilities for converting from platform-specific objects (iOS
 objects) into generic Viro objects.
 */
class VROConvert {
    
public:
    
    static VROMatrix4f toMatrix4f(GLKMatrix4 glm);
    
};

#endif /* VROConvert_h */
