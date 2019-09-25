//
//  VROGeometry.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROGeometry.h"
#include "VROGeometrySource.h"
#include "VROGeometrySubstrate.h"
#include "VROGeometryElement.h"
#include "VROLog.h"
#include "VROLight.h"
#include "VRONode.h"
#include "VROGeometryUtil.h"
#include "VROMaterial.h"
#include "VRORenderMetadata.h"
#include "VROMorpher.h"

VROGeometry::~VROGeometry() {
    delete (_substrate);
    ALLOCATION_TRACKER_SUB(Geometry, 1);
}

void VROGeometry::deleteGL() {
    for (std::shared_ptr<VROMaterial> &material : _materials) {
        material->deleteGL();
    }
}

void VROGeometry::prewarm(std::shared_ptr<VRODriver> driver) {
    if (!_substrate && isRenderable()) {
        _substrate = driver->newGeometrySubstrate(*this);
    }
}

void VROGeometry::render(int elementIndex,
                         const std::shared_ptr<VROMaterial> &material,
                         VROMatrix4f transform,
                         VROMatrix4f normalMatrix,
                         float opacity,
                         const VRORenderContext &context,
                         std::shared_ptr<VRODriver> &driver) {
    prewarm(driver);
    if (_substrate) {
        _substrate->render(*this, elementIndex, transform, normalMatrix,
                           opacity, material, context, driver);
    }
}

void VROGeometry::renderSilhouette(VROMatrix4f transform,
                                   std::shared_ptr<VROMaterial> &material,
                                   const VRORenderContext &context,
                                   std::shared_ptr<VRODriver> &driver) {
    prewarm(driver);
    if (_substrate) {
        _substrate->renderSilhouette(*this, transform, material, context, driver);
    }
}

void VROGeometry::renderSilhouetteTextured(int element,
                                           VROMatrix4f transform,
                                           std::shared_ptr<VROMaterial> &material,
                                           const VRORenderContext &context,
                                           std::shared_ptr<VRODriver> &driver) {
    prewarm(driver);
    if (_substrate) {
        _substrate->renderSilhouetteTextured(*this, element, transform, material, context, driver);
    }
}

void VROGeometry::updateSortKeys(VRONode *node, uint32_t hierarchyId, uint32_t hierarchyDepth,
                                 uint32_t lightsHash, const std::vector<std::shared_ptr<VROLight>> &lights,
                                 float opacity, float distanceFromCamera, float zFar,
                                 std::shared_ptr<VRORenderMetadata> &metadata,
                                 const VRORenderContext &context,
                                 std::shared_ptr<VRODriver> &driver) {
    _sortKeys.clear();

    size_t numElements = _geometryElements.size();
    for (size_t i = 0; i < numElements; i++) {
        int materialIndex = i % (int) _materials.size();
        
        VROSortKey key;
        key.renderingOrder = node->getRenderingOrder();
        key.hierarchyId = kMaxHierarchyId - hierarchyId;
        key.hierarchyDepth = hierarchyDepth;
        key.lights = lightsHash;
        key.node = (uintptr_t) node;
        key.elementIndex = (int) i;
        key.distanceFromCamera = zFar - distanceFromCamera;
        
        std::shared_ptr<VROMaterial> &material = _materials[materialIndex];
        material->updateSortKey(key, lights, context, driver);

        key.transparent = (node->getOpacity() < (1 - kEpsilon) ||
                           material->getTransparency() < (1 - kEpsilon) ||
                           material->hasDiffuseAlpha());
        key.incoming = true;
        
        _sortKeys.push_back(key);
        
        const std::shared_ptr<VROMaterial> &outgoing = material->getOutgoing();
        if (outgoing) {
            outgoing->updateSortKey(key, lights, context, driver);
            key.incoming = false;
            
            _sortKeys.push_back(key);
        }
        
        if (material->isBloomSupported()) {
            metadata->setRequiresBloomPass(true);
        }

        if (material->getPostProcessMask()) {
            metadata->setRequiresPostProcessMaskPass(true);
        }
    }
    
    if (_substrate) {
        bool updatedMorphSources = false;
        for (int i = 0; i < _elementsToMorphers.size(); i ++) {
            if (_elementsToMorphers[i]->update(_geometrySources)){
                updatedMorphSources = true;
            }
        }

        // Process morph target data, if any.
        if (updatedMorphSources) {
            setSources(_geometrySources);
            prewarm(driver);
        }

        _substrate->update(*this, driver);
    }
}

void VROGeometry::getSortKeys(std::vector<VROSortKey> *outKeys) {
    outKeys->insert(outKeys->end(), _sortKeys.begin(), _sortKeys.end());
}

const VROBoundingBox &VROGeometry::getBoundingBox() {
    if (_boundingBoxComputed) {
        return _bounds;
    }
    
    auto vertexSources = getGeometrySourcesForSemantic(VROGeometrySourceSemantic::Vertex);
    for (std::shared_ptr<VROGeometrySource> &source : vertexSources) {
        VROBoundingBox box = source->getBoundingBox();
        
        if (!_boundingBoxComputed) {
            _bounds = box;
            _boundingBoxComputed = true;
        }
        else {
            _bounds.unionDestructive(box);
        }
    }
    _boundingBoxComputed = true;
    return _bounds;
}

VROVector3f VROGeometry::getCenter() {
    return getBoundingBox().getCenter();
}

void VROGeometry::setGeometrySourceForSemantic(VROGeometrySourceSemantic semantic,
                                               std::shared_ptr<VROGeometrySource> source) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    for (const std::shared_ptr<VROGeometrySource> &source : _geometrySources) {
        if (source->getSemantic() != semantic) {
            sources.push_back(source);
        }
    }
    sources.push_back(source);
    setSources(sources);
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

bool VROGeometry::isRenderable() const {
    if (_geometrySources.empty()) {
        return false;
    }
    if (_geometryElements.empty()) {
        return false;
    }

    // At least one element should have data
    for (const std::shared_ptr<VROGeometryElement> &element : _geometryElements) {
        if (element->getData() != nullptr) {
            return true;
        }
    }
    return false;
}

void VROGeometry::updateSubstrate() {
    delete (_substrate);
    _substrate = nullptr;
}

void VROGeometry::updateBoundingBox(){
    _boundingBoxComputed = false;
}
