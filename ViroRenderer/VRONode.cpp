//
//  VRONode.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRONode.h"

VRONode::VRONode(const VRORenderContext &context) {
    
}

VRONode::~VRONode() {
    
}

void VRONode::render(const VRORenderContext &context, std::stack<VROMatrix4f> xforms) {
    VROMatrix4f transform = xforms.top().multiply(_transform);
    
    if (_geometry) {
        _geometry->render(context, transform);
    }
    
    xforms.push(transform);
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, xforms);
    }
    
    xforms.pop();
}