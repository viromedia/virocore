//
//  VROGeometry.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometry.h"
#include "VROGeometrySource.h"
#include "VROGeometrySubstrate.h"
#include "VRORenderParameters.h"

VROGeometry::~VROGeometry() {
    delete (_bounds);
    delete (_substrate);
}

void VROGeometry::render(const VRORenderContext &context,
                         VRORenderParameters &params) {
    
    if (!_substrate) {
        _substrate = context.newGeometrySubstrate(*this);
    }
    _substrate->render(_materials, context, params);
}

const VROBoundingBox &VROGeometry::getBoundingBox() {
    if (_bounds) {
        return *_bounds;
    }
    
    auto vertexSources = getGeometrySourcesForSemantic(VROGeometrySourceSemantic::Vertex);
    for (std::shared_ptr<VROGeometrySource> &source : vertexSources) {
        VROBoundingBox box = source->getBoundingBox();
        
        if (!_bounds) {
            _bounds = new VROBoundingBox(box);
        }
        else {
            _bounds->unionDestructive(box);
        }
    }
    
    return *_bounds;
}

std::vector<std::shared_ptr<VROGeometrySource>> VROGeometry::getGeometrySourcesForSemantic(VROGeometrySourceSemantic semantic) const {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    
    for (const std::shared_ptr<VROGeometrySource> &source : _geometrySources) {
        if (source->getSemantic() == semantic) {
            sources.push_back(source);
        }
    }
    
    return sources;
}
