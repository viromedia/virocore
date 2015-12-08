//
//  VRONode.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRONode.h"
#include "VROGeometry.h"
#include "VROLight.h"

VRONode::VRONode(const VRORenderContext &context) {
    
}

VRONode::~VRONode() {
    
}

void VRONode::render(const VRORenderContext  &context,
                     std::stack<VROMatrix4f> &xforms,
                     std::vector<std::shared_ptr<VROLight>> &lights) {
    
    if (_light) {
        lights.push_back(_light);
    }
    
    VROMatrix4f transform = xforms.top().multiply(_transform);
    if (_geometry) {
        _geometry->render(context, transform, lights);
    }
    
    xforms.push(transform);
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, xforms, lights);
    }
    
    xforms.pop();
    if (_light) {
        lights.pop_back();
    }
}