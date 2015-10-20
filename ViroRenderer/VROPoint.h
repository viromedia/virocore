//
//  VROPoint.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROPoint_h
#define VROPoint_h

class VROPoint {
    
public:
    
    float x, y, z;
    
    VROPoint() :
        x(0), y(0), z(0)
    {}
    
    VROPoint(float x, float y) :
        x(x), y(y), z(0)
    {}
    
    VROPoint(float x, float y, float z) :
        x(x), y(y), z(z)
    {}
    
};

#endif /* VROPoint_h */
