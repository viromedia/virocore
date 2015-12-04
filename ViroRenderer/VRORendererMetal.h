//
//  VRORendererMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORendererMetal_h
#define VRORendererMetal_h

#include "VRORenderer.h"

class VRORendererMetal : public VRORenderer {
    
public:
    
    void render(std::shared_ptr<VROScene> scene);
    
};

#endif /* VRORendererMetal_h */
