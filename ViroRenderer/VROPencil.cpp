//
//  VROPencil.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPhysicsBody.h"
#include "VRONode.h"
#include "VROPolyline.h"
#include "VROMaterial.h"

VROPencil::~VROPencil() {
    _paths.clear();
}

void VROPencil::draw(const VROVector3f from, const VROVector3f to) {
    std::vector<VROVector3f> pathTot;
    pathTot.push_back(from);
    pathTot.push_back(to);
    _paths.push_back(pathTot);
}

void VROPencil::clear() {
    _paths.clear();
}

void VROPencil::render(const VRORenderContext &renderContext, std::shared_ptr<VRODriver> &driver) {
    if (_paths.size() == 0) {
        return;
    }

    // Creates a single VROPolyline that represents a vector of paths
    // with a preset width and color.
    std::shared_ptr<VROPolyline> line = VROPolyline::createPolyline(_paths, 0.05f);
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setColor({255, 0, 0, 1.0});
    material->setCullMode(VROCullMode::None);
    line->setMaterials({ material });

    VROMatrix4f parentTransform;
    line->render(0, material, parentTransform,
           parentTransform.invert().transpose(), 1.0, renderContext, driver);
}
