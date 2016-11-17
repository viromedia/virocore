//
//  VROSample.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include "Viro.h"
#include "VROSample.h"
#include "VRORenderer.h"
#include "VRODriverOpenGLAndroid.h"
#include "VROImageAndroid.h"
#include "VROSceneRendererCardboard.h"
#include "VROVideoTextureAndroid.h"
#include <chrono>
#include <ctime>

VROSample::VROSample() {

}

VROSample::~VROSample() {

}

std::shared_ptr<VROSceneController> VROSample::loadBoxScene(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                            VRODriver &driver) {

    _driver = &driver;
    frameSynchronizer->addFrameListener(shared_from_this());

    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube(getNiagaraTexture());

    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.4, 0.4, 0.4 });

    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.0, 0.0 });
    spotRed->setPosition( { -5, 0, 0 });
    spotRed->setDirection( { 1.0, 0, -1.0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(2.5);
    spotRed->setSpotOuterAngle(5.0);

    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(2.5);
    spotBlue->setSpotOuterAngle(5.0);

    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);

    scene->addNode(rootNode);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");

    _videoA = std::make_shared<VROVideoTextureAndroid>();
    _videoA->loadVideoFromAsset("vest.mp4", driver);
    _videoA->play();

    _material = box->getMaterials()[0];
    _material->setLightingModel(VROLightingModel::Phong);
    _material->getDiffuse().setContents(_videoA);
    _material->getSpecular().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("specular.png")));

    std::vector<std::string> modifierCode =  { "uniform float testA;",
                                               "uniform float testB;",
                                               "_geometry.position.x = _geometry.position.x + testA;"
    };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);

    modifier->setUniformBinder("testA", [](VROUniform *uniform, GLuint location) {
        uniform->setFloat(1.0);
    });
    _material->addShaderModifier(modifier);

    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});

    rootNode->addChildNode(boxNode);
    //boxNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));

    /*
     Create a second box node behind the first.
     */
    std::shared_ptr<VROBox> box2 = VROBox::createBox(2, 4, 2);
    box2->setName("Box 2");

    std::shared_ptr<VROMaterial> material2 = box2->getMaterials()[0];
    material2->setLightingModel(VROLightingModel::Phong);
    material2->getDiffuse().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("boba.png")));
    material2->getSpecular().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("specular.png")));

    std::shared_ptr<VRONode> boxNode2 = std::make_shared<VRONode>();
    boxNode2->setGeometry(box2);
    boxNode2->setPosition({0, 0, -9});
    boxNode2->addLight(ambient);

    //rootNode->addChildNode(boxNode2);

    //[self.view setCameraRotationType:VROCameraRotationType::Orbit];
    //[self.view setOrbitFocalPoint:boxNode->getPosition()];

    VROTransaction::begin();
    VROTransaction::setAnimationDuration(6);

    spotRed->setPosition({5, 0, 0});
    spotRed->setDirection({-1, 0, -1});

    spotBlue->setPosition({-5, 0, 0});
    spotBlue->setDirection({1, 0, -1});

    VROTransaction::commit();

    return sceneController;
}

std::shared_ptr<VROTexture> VROSample::getNiagaraTexture() {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {
            std::make_shared<VROImageAndroid>("px.png"),
            std::make_shared<VROImageAndroid>("nx.png"),
            std::make_shared<VROImageAndroid>("py.png"),
            std::make_shared<VROImageAndroid>("ny.png"),
            std::make_shared<VROImageAndroid>("pz.png"),
            std::make_shared<VROImageAndroid>("nz.png")
    };

    return std::make_shared<VROTexture>(cubeImages);
}

// Test changing the video after a given number of seconds
std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
static bool isSet = false;

void VROSample::onFrameWillRender(const VRORenderContext &context) {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = now - start;

    if (elapsed_seconds.count() > 10 && !isSet) {
        _videoA->loadVideo("http://s3-us-west-2.amazonaws.com/viro/360_surf.mp4", {}, *_driver);
        _videoA->play();
        isSet = true;
    }
}

void VROSample::onFrameDidRender(const VRORenderContext &context) {

}