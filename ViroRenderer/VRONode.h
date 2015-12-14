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
#include "VRORenderContext.h"

class VROGeometry;
class VROLight;

class VRONode : public std::enable_shared_from_this<VRONode> {
    
public:
    
    /*
     Designated initializer for nodes in the model tree.
     */
    VRONode(const VRORenderContext &context);
    
    /*
     Designated initializer for nodes in the presentation tree.
     */
    VRONode();
    virtual ~VRONode();
    
    void render(const VRORenderContext  &context,
                std::stack<VROMatrix4f> &rotations,
                std::stack<VROMatrix4f> &xforms,
                std::vector<std::shared_ptr<VROLight>> &lights);
    
    void setGeometry(std::shared_ptr<VROGeometry> geometry) {
        _geometry = geometry;
    }
    std::shared_ptr<VROGeometry> getGeometry() const {
        return _geometry;
    }
    
    VROMatrix4f getTransform() const;
    
    void setRotation(VROQuaternion rotation) {
        _rotation = rotation;
    }
    void setPosition(VROVector3f position) {
        _position = position;
    }
    void setScale(VROVector3f scale) {
        _scale = scale;
    }
    
    void setLight(std::shared_ptr<VROLight> light) {
        _light = light;
    }
    std::shared_ptr<VROLight> getLight() {
        return _light;
    }
    
    void addChildNode(std::shared_ptr<VRONode> node) {
        _subnodes.push_back(node);
        node->_supernode = shared_from_this();
    }
    void removeFromParentNode() {
        std::vector<std::shared_ptr<VRONode>> parentSubnodes = _supernode->_subnodes;
        parentSubnodes.erase(
                             std::remove_if(parentSubnodes.begin(), parentSubnodes.end(),
                                            [this](std::shared_ptr<VRONode> layer) {
                                                return layer.get() == this;
                                            }), parentSubnodes.end());
        _supernode.reset();
    }
    
protected:
    
    /*
     The node's parent and children.
     */
    std::vector<std::shared_ptr<VRONode>> _subnodes;
    std::shared_ptr<VRONode> _supernode;
    
private:
    
    std::shared_ptr<VROGeometry> _geometry;
    std::shared_ptr<VROLight> _light;
    
    VROVector3f _scale;
    VROVector3f _position;
    VROQuaternion _rotation;
    
    /*
     The 'presentation' counterpart of this node. The presentation node
     reflects what's actually on the screen during an animation, as opposed
     to the model.
     */
    std::shared_ptr<VRONode> _presentationNode;

};

#endif /* VRONode_h */
