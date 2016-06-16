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
#include "VROLight.h"
#include "VROGeometryUtil.h"
#include "VROMaterial.h"

VROGeometry::~VROGeometry() {
    delete (_bounds);
    delete (_substrate);
    
    ALLOCATION_TRACKER_SUB(Geometry, 1);
}

void VROGeometry::prewarm(const VRODriver &driver) {
    if (!_substrate) {
        _substrate = driver.newGeometrySubstrate(*this);
    }
}

void VROGeometry::render(const VRORenderContext &renderContext,
                         const VRODriver &driver,
                         VRORenderParameters &params) {
    
    prewarm(driver);
    _substrate->render(*this, _materials, renderContext, driver, params);
}

void VROGeometry::render(int elementIndex,
                         std::shared_ptr<VROMaterial> &material,
                         VROMatrix4f transform,
                         float opacity,
                         const std::vector<std::shared_ptr<VROLight>> &lights,
                         const VRORenderContext &context,
                         const VRODriver &driver) {
    
    prewarm(driver);
    _substrate->render(*this, elementIndex, transform, opacity, material, lights, context, driver);
}

void VROGeometry::updateSortKeys(VRONode *node, uint32_t lightsHash) {
    _sortKeys.clear();
    
    size_t numElements = _geometryElements.size();
    for (size_t i = 0; i < numElements; i++) {
        int materialIndex = i % (int) _materials.size();
        
        VROSortKey key;
        key.renderingOrder = _renderingOrder;
        key.lights = lightsHash;
        key.node = (uintptr_t) node;
        key.elementIndex = i;
        
        std::shared_ptr<VROMaterial> &material = _materials[materialIndex];
        material->updateSortKey(key);
        key.outgoing = false;
        
        _sortKeys.push_back(key);
        
        const std::shared_ptr<VROMaterial> &outgoing = material->getOutgoing();
        if (outgoing) {
            outgoing->updateSortKey(key);
            key.outgoing = true;
            
            _sortKeys.push_back(key);
        }
    }
}

void VROGeometry::getSortKeys(std::vector<VROSortKey> *outKeys) {
    outKeys->insert(outKeys->end(), _sortKeys.begin(), _sortKeys.end());
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

