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
#include "VROAllocationTracker.h"

class VRONode;
class VRORenderContext;
class VROTexture;
class VROGeometry;

class VROScene {
    
public:
    
    VROScene();
    virtual ~VROScene();
    
    void renderBackground(const VRORenderContext &context);
    void render(const VRORenderContext &context);
    
    /*
     Add a new root node to the scene.
     */
    void addNode(std::shared_ptr<VRONode> node);
    std::vector<std::shared_ptr<VRONode>> &getRootNodes() {
        return _nodes;
    }
    
    /*
     Set the background of the scene to a cube-map defined by
     the given cube texture.
     */
    void setBackground(std::shared_ptr<VROTexture> textureCube);
    std::shared_ptr<VROGeometry> getBackground() const {
        return _background;
    }
    
private:
    
    /*
     The root nodes of the scene.
     */
    std::vector<std::shared_ptr<VRONode>> _nodes;
    
    /*
     The background visual to display. Rendered before any nodes.
     */
    std::shared_ptr<VROGeometry> _background;
    
};

#endif /* VROScene_h */
