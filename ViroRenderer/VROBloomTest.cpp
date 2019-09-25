//
//  VROBloomTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
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

#include "VROBloomTest.h"
#include "VROTestUtil.h"
#include "VRORenderer.h"
#include "VROGaussianBlurRenderPass.h"
#include "VROChoreographer.h"
#include "VROTypeface.h"
#include "VROPolyline.h"
#include "VROBillboardConstraint.h"

VROBloomTest::VROBloomTest() :
    VRORendererTest(VRORendererTestType::Bloom) {

}

VROBloomTest::~VROBloomTest() {

}

void VROBloomTest::build(std::shared_ptr<VRORenderer> renderer,
                         std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                         std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _renderer->setDebugHUDEnabled(true);
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    setEnabledEvent(EventAction::OnClick, true);

    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    scene->getRootNode()->setCamera(camera);
    _pointOfView = scene->getRootNode();

    std::shared_ptr<VROLight> amLight = std::make_shared<VROLight>(VROLightType::Ambient);
    amLight->setIntensity(20);
    _pointOfView->addLight(amLight);

    camera->setPosition({ 2, 2, 2 });
    camera->setBaseRotation({ 0, M_PI_2 / 2, 0 });

    _kernelSize = 3;
    _sigma = 0.1;
    _useBilinearTextureLookup = true;
    _blurPasses = 4;
    _bloomStateText = VROText::createText(L"< Bloom State >", "Helvetica", 21,
                                             VROFontStyle::Normal, VROFontWeight::Regular, {1.0, 1.0, 1.0, 1.0}, 0, 5.2, 0.2,
                                             VROTextHorizontalAlignment::Center, VROTextVerticalAlignment::Center,
                                             VROLineBreakMode::WordWrap, VROTextClipMode::None, 0, driver);
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(_bloomStateText);
    textNode->setPosition({ -1, 2.5, -3 });
    textNode->setScale({2,2,2});
    rootNode->addChildNode(textNode);


    std::shared_ptr<VROPolyline> polyline = std::make_shared<VROPolyline>();
    polyline->setThickness(0.10);
    polyline->getMaterials()[0]->setLightingModel(VROLightingModel::Constant);
    polyline->getMaterials()[0]->getDiffuse().setColor({1.0,0.0,0.0, 1});
    polyline->getMaterials()[0]->setBloomThreshold(0.0);

    polyline->appendPoint({ -1, 0, 0 });
    polyline->appendPoint({ 0, 0, 0 });
    polyline->appendPoint({ 1, 0, 0 });
    polyline->appendPoint({ 1, 1, 0 });

    std::shared_ptr<VRONode> polylineNode = std::make_shared<VRONode>();
    polylineNode->setIgnoreEventHandling(true);
    polylineNode->setGeometry(polyline);
    polylineNode->setScale({ 1,1,1});
    polylineNode->setPosition({ -4.75, 1.5, -0.75 });
    polylineNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    rootNode->addChildNode(polylineNode);

    std::vector<VROVector3f> lightPositions;
    lightPositions.push_back({ -2.0f, 1.5f, 1.5f});
    lightPositions.push_back({-4.0f, 1.8f, -3.0f});
    lightPositions.push_back({ 3.0f, 1.2f,  1.0f});
    lightPositions.push_back({.8f,  1.4f, -1.0f});

    std::vector<VROVector3f> lightColors;
    lightColors.push_back({5.0, 5.0, 5.0});
    lightColors.push_back({5.0, 0.0, 0.0});
    lightColors.push_back({0.0, 5.0, 0.0});
    lightColors.push_back({0.0, 0.0, 5.0});

    for (int i = 0; i < lightPositions.size(); i++) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
        light->setColor(lightColors[i]);
        light->setPosition({ lightPositions[i].x, lightPositions[i].y, lightPositions[i].z});
        light->setAttenuationStartDistance(0);
        light->setAttenuationEndDistance(5);
        rootNode->addLight(light);
    }

    std::shared_ptr<VROTexture> woodTexture = VROTestUtil::loadDiffuseTexture("wood");
    woodTexture->setWrapS(VROWrapMode::Repeat);
    woodTexture->setWrapT(VROWrapMode::Repeat);
    woodTexture->setMinificationFilter(VROFilterMode::Linear);
    woodTexture->setMagnificationFilter(VROFilterMode::Linear);
    woodTexture->setMipFilter(VROFilterMode::Linear);

    std::vector<VROVector3f> boxPositions;
    boxPositions.push_back({ 0,  -2, 0 });
    boxPositions.push_back({ 0, 4.5, 0 });
    boxPositions.push_back({ 2, 0, 1 });
    boxPositions.push_back({ 3, -1, 2 });
    boxPositions.push_back({ 0, 2.7, 4 });
    boxPositions.push_back({ -2, -1, -3 });
    boxPositions.push_back({ -3, 0, 0 });

    std::vector<VROVector3f> boxScales;
    boxScales.push_back({12.5, 0.5, 12.5});
    boxScales.push_back({0.5, 0.5, 0.5});
    boxScales.push_back({0.5, 0.5, 0.5});
    boxScales.push_back({1, 1, 1});
    boxScales.push_back({1.25, 1.25, 1.25});
    boxScales.push_back({1, 1, 1});
    boxScales.push_back({0.5, 0.5, 0.5});

    std::vector<VROQuaternion> boxRotations;
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({1, 0, 1, toRadians(60)});
    boxRotations.push_back({1, 0, 1, toRadians(23)});
    boxRotations.push_back({1, 0, 1, toRadians(145)});
    boxRotations.push_back({0, 1, 0, 0});

    for (int i = 0; i < boxPositions.size(); i++) {
        std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);

        std::shared_ptr<VROMaterial> boxMaterial = box->getMaterials()[0];
        boxMaterial->setLightingModel(VROLightingModel::Lambert);
        boxMaterial->getDiffuse().setTexture(woodTexture);

        std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
        boxNode->setGeometry(box);
        boxNode->setPosition({ boxPositions[i].x, boxPositions[i].y, boxPositions[i].z });
        boxNode->setScale(boxScales[i]);
        //boxNode->setRotation(boxRotations[i]);
        rootNode->addChildNode(boxNode);
        boxNode->setName("Box: " + VROStringUtil::toString(i));
        boxNode->setEventDelegate(shared_from_this());

    }

    std::vector<std::shared_ptr<VROMaterial>> boxMaterials;
    for (int i = 0; i < lightPositions.size(); i++) {
        std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);

        std::shared_ptr<VROMaterial> boxMaterial = box->getMaterials()[0];
        boxMaterial->setLightingModel(VROLightingModel::Constant);
        boxMaterial->getDiffuse().setColor({lightColors[i].x, lightColors[i].y, lightColors[i].z, 1});
        boxMaterial->setBloomThreshold(0.0);

        std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
        boxNode->setGeometry(box);
        boxNode->setPosition({ lightPositions[i].x, lightPositions[i].y, lightPositions[i].z });
        boxNode->setScale({ 0.25, 0.25, 0.25 });
        rootNode->addChildNode(boxNode);
        boxMaterials.push_back(boxMaterial);
        boxNode->setName("BoxGlow: " + VROStringUtil::toString(i));
        boxNode->setEventDelegate(shared_from_this());
    }
}

void VROBloomTest::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position){
    std::shared_ptr<VROGaussianBlurRenderPass> pass =
            _renderer->getChoreographer()->getGaussianBlurPass();
    if (clickState == ClickDown) {
        std::string name = node->getName();
        if (VROStringUtil::strcmpinsensitive(name, "Box: 5")){
            _sigma = _sigma + 0.5;
        } else if (VROStringUtil::strcmpinsensitive(name, "Box: 6")){
            _kernelSize = _kernelSize + 2;
        } else if (VROStringUtil::strcmpinsensitive(name, "Box: 0")){
            _useBilinearTextureLookup = !_useBilinearTextureLookup;
            pass->setBilinearTextureLookup(_useBilinearTextureLookup);
        }  else if (VROStringUtil::strcmpinsensitive(name, "Box: 2")){
            _blurPasses = _blurPasses + 2;
            if (_blurPasses > 20) {
                _blurPasses = 4;
            }
            return;
        }

        if (_kernelSize > 223) {
            _kernelSize = 3;
        }

        if (_sigma > 51) {
            _sigma = 0.1;
        }
        std::string bloomText = "Sigma: " + VROStringUtil::toString(_sigma, 4) +
                                " Kernel: " + VROStringUtil::toString(_kernelSize) +
                                " T: " + VROStringUtil::toString(_useBilinearTextureLookup);
        std::wstring wsTmp(bloomText.begin(), bloomText.end());
        _bloomStateText->setText(wsTmp);
        pass->setBlurKernel(_kernelSize, _sigma, false);
    }
}
