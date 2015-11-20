//
//  VRORenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORenderer_h
#define VRORenderer_h

#include <stdio.h>
#include <memory>
#include "VROScene.h"

class VRORenderer {
    
public:
    
    void render(std::shared_ptr<VROScene> scene);
    
private:
    
    
};

#endif /* VRORenderer_h */
