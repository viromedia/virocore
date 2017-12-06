//
//  VROPencil.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPencil.h"
#include "VROPhysicsBody.h"
#include "VRONode.h"
#include "VROPolyline.h"
#include "VROMaterial.h"

std::shared_ptr<VROMaterial> sPencilMaterial;

VROPencil::~VROPencil() {
    _paths.clear();
}

void VROPencil::draw(VROVector3f from, VROVector3f to) {
    std::vector<VROVector3f> pathTot;
    pathTot.push_back(from);
    pathTot.push_back(to);
    _paths.push_back(pathTot);
}

void VROPencil::clear() {
    _paths.clear();
}

void VROPencil::render(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_paths.size() == 0) {
        return;
    }

    if (!sPencilMaterial) {
        sPencilMaterial = std::make_shared<VROMaterial>();
        sPencilMaterial->getDiffuse().setColor({1.0, 0, 0, 1.0});
        sPencilMaterial->setCullMode(VROCullMode::None);
        sPencilMaterial->setLightingModel(VROLightingModel::Constant);
        sPencilMaterial->setWritesToDepthBuffer(false);
        sPencilMaterial->setReadsFromDepthBuffer(false);
        sPencilMaterial->bindShader(0, {}, driver);
        sPencilMaterial->bindProperties(driver);
    }

    std::shared_ptr<VROPolyline> line = VROPolyline::createPolyline(_paths, 0.05f);
    line->setMaterials({ sPencilMaterial });

    sPencilMaterial->bindShader(0, {}, driver);
    sPencilMaterial->bindProperties(driver);
    line->render(0, sPencilMaterial, VROMatrix4f::identity(), VROMatrix4f::identity(),
                 1.0, context, driver);
}
