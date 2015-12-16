//
//  VROInterpolatable.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROInterpolatable_h
#define VROInterpolatable_h


template <class T> class VROInterpolatable {

public:
    
    virtual T interpolate(T x, float t) = 0;
    
};

#endif /* VROInterpolatable_h */
