//
//  VROScene.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/19/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROScene_h
#define VROScene_h

#include <stdio.h>
#include <vector>
#include <memory>

class VROLayer;
class VRORenderContext;

class VROScene {
    
public:
    
    void render(const VRORenderContext &renderContext);
    
    void addLayer(std::shared_ptr<VROLayer> layer);
    
private:
    
    std::vector<std::shared_ptr<VROLayer>> _layers;
    
};

#endif /* VROScene_h */
