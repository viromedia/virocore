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
#include "VROGeometryElement.h"
#include "VROLog.h"
#include "VROGeometryUtil.h"

VROGeometry::~VROGeometry() {
    delete (_bounds);
    delete (_substrate);
    
    ALLOCATION_TRACKER_SUB(Geometry, 1);
}

void VROGeometry::prewarm(const VRORenderContext &context) {
    if (!_substrate) {
        _substrate = context.newGeometrySubstrate(*this);
    }
}

void VROGeometry::render(const VRORenderContext &context,
                         VRORenderParameters &params) {
    
    prewarm(context);
    _substrate->render(*this, _materials, context, params);
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

VROVector3f VROGeometry::getCenter() {
    return getBoundingBox().getCenter();
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

