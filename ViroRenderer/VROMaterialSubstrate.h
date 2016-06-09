//
//  VROMaterialSubstrate.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/29/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrate_h
#define VROMaterialSubstrate_h

#include <stdio.h>

class VROSortKey;

class VROMaterialSubstrate {
public:
    virtual ~VROMaterialSubstrate() {}
    
    virtual void updateSortKey(VROSortKey &key) const = 0;
    
};

#endif /* VROMaterialSubstrate_h */
