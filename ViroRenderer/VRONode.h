//
//  VRONode.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRONode_h
#define VRONode_h

#include <memory>
#include <stack>
#include <vector>
#include "VROMatrix4f.h"
#include "VROQuaternion.h"
#include "VROGeometry.h"
#include "VRORenderContext.h"

class VRONode {
    
public:
    
    /*
     Designated initializer for nodes in the model tree.
     */
    VRONode(const VRORenderContext &context);
    
    /*
     Designated initializer for nodes in the presentation tree.
     */
    VRONode(VRONode *layer);
    virtual ~VRONode();
    
    virtual void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    
    void setGeometry(std::shared_ptr<VROGeometry> geometry) {
        _geometry = geometry;
    }
    std::shared_ptr<VROGeometry> getGeometry() const {
        return _geometry;
    }
    
    VROMatrix4f getTransform() const {
        return _transform;
    }
    void setTransform(VROMatrix4f transform) {
        _transform = transform;
    }
    
protected:
    
    /*
     The node's parent and children.
     */
    std::vector<std::shared_ptr<VRONode>> _subnodes;
    std::shared_ptr<VRONode> _supernode;
    
private:
    
    std::shared_ptr<VROGeometry> _geometry;
    
    VROVector3f _position;
    VROMatrix4f _transform;
    VROQuaternion _rotation;
    
    /*
     The 'presentation' counterpart of this node. The presentation node
     reflects what's actually on the screen during an animation, as opposed
     to the model.
     */
    std::shared_ptr<VRONode> _presentationNode;
    
};

#endif /* VRONode_h */
