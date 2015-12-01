//
//  VRONode.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRONode.h"

void VRONode::render(const VRORenderContext &context, std::stack<VROMatrix4f> xforms) {
    if (_geometry) {
        renderGeometry();
    }
    
    xforms.push(xforms.top().multiply(_transform));
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, xforms);
    }
    
    xforms.pop();
}

void VRONode::renderGeometry() {
    std::vector<std::shared_ptr<VROGeometrySource>> sources   = _geometry->getGeometrySources();
    std::vector<std::shared_ptr<VROGeometryElement>> elements = _geometry->getGeometryElements();
    std::vector<std::shared_ptr<VROMaterial>> materials       = _geometry->getMaterials();
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        
    }
}