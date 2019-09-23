//
//  VROPencil.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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
