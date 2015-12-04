//
//  VRORendererMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRORendererMetal.h"

void VRORendererMetal::render(std::shared_ptr<VROScene> scene) {
    std::vector<std::shared_ptr<VRONode>> &nodes = scene->getRootNodes();
    
    for (std::shared_ptr<VRONode> node : nodes) {
        
    }
}
