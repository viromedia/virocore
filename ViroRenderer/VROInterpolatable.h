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


//class VROInterpolatable {
    
//};

//template <class A_Type> A_Type calc<A_Type>::multiply(A_Type x,A_Type y)
//{
//    return x*y;
//}

#endif /* VROInterpolatable_h */
