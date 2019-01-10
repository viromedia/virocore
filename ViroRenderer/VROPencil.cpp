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

    std::shared_ptr<VROMaterial> pencilMaterial = std::make_shared<VROMaterial>();
    pencilMaterial->getDiffuse().setColor({1.0, 0, 0, 1.0});
    pencilMaterial->setCullMode(VROCullMode::None);
    pencilMaterial->setLightingModel(VROLightingModel::Constant);
    pencilMaterial->setWritesToDepthBuffer(false);
    pencilMaterial->setReadsFromDepthBuffer(false);
    pencilMaterial->bindShader(0, {}, context, driver);
    pencilMaterial->bindProperties(driver);

    std::shared_ptr<VROPolyline> line = VROPolyline::createPolyline(_paths, _brushThickness);
    line->setMaterials({ pencilMaterial });

    pencilMaterial->bindShader(0, {}, context, driver);
    pencilMaterial->bindProperties(driver);
    line->render(0, pencilMaterial, VROMatrix4f::identity(), VROMatrix4f::identity(),
                 1.0, context, driver);
}
