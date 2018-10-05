//
//  VROTextureSubstrate.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTextureSubstrate_h
#define VROTextureSubstrate_h

#include <stdio.h>

enum class VROWrapMode;

class VROTextureSubstrate {
public:
    virtual ~VROTextureSubstrate() {}
    virtual void updateWrapMode(VROWrapMode wrapModeS, VROWrapMode wrapModeT) = 0;
};

#endif /* VROTextureSubstrate_h */
