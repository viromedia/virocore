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
#include "VROVideoTextureAVP.h"
#include "VROText.h"
#include "VROPlatformUtil.h"
#include "VROOBJLoader.h"

VROSample::VROSample() {

}

VROSample::~VROSample() {

}

std::shared_ptr<VROSceneController> VROSample::loadBoxScene(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                            VRODriver &driver) {


    _driver = &driver;
    frameSynchronizer->addFrameListener(shared_from_this());

    _soundEffect = driver.newSoundEffect("btn_tap.mp3");
    _audio = driver.newAudioPlayer("underwater.mp3");

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
     Create the obj node.
     */
    std::string heartPath = VROPlatformCopyAssetToFile("heart.obj");
    std::shared_ptr<VRONode> heartNode = VROOBJLoader::loadOBJFromFile(heartPath, "", true, [](std::shared_ptr<VRONode> node, bool success) {
        if (!success) {
            return;
        }

        std::shared_ptr<VROMaterial> &material = node->getGeometry()->getMaterials().front();
        material->getDiffuse().setTexture(std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("heart_d.jpg")));
        material->getSpecular().setTexture(std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("heart_s.jpg")));
        material->setLightingModel(VROLightingModel::Blinn);
    });

    heartNode->setPosition({0, -5.25, -1});
    rootNode->addChildNode(heartNode);

    /*
     Create the 007 node. Copy all the required assets into the cache dir so they
     can be accessed by the OBJ loader.
     */
    std::string malePath = VROPlatformCopyAssetToFile("male02.obj");
    VROPlatformCopyAssetToFile("male02.mtl");
    VROPlatformCopyAssetToFile("male-02-1noCulling.JPG");
    VROPlatformCopyAssetToFile("orig_02_-_Defaul1noCulling.JPG");
    VROPlatformCopyAssetToFile("01_-_Default1noCulling.JPG");

    std::string base = malePath.substr(0, malePath.find_last_of('/'));

    std::shared_ptr<VRONode> maleNode = VROOBJLoader::loadOBJFromFile(malePath, base, true);
    maleNode->setPosition({0, -100, -10});
    maleNode->setScale({ 0.1, 0.1, 0.1 });
    rootNode->addChildNode(maleNode);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");

    _videoA = std::make_shared<VROVideoTextureAVP>();
    _videoA->loadVideoFromAsset("vest.mp4", driver);
    _videoA->setLoop(true);
    //_videoA->play();

    _material = box->getMaterials()[0];
    _material->setLightingModel(VROLightingModel::Lambert);
    _material->getDiffuse().setTexture(_videoA);
    //_material->getDiffuse().setTexture(std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("boba.png")));
    _material->getSpecular().setTexture(std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("specular.png")));

    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -15});

    rootNode->addChildNode(boxNode);

    std::string string = "In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    std::shared_ptr<VROTypeface> typeface = driver.newTypeface("Roboto", 10);
    std::shared_ptr<VROText> text = VROText::createText(string, typeface, {1.0, 1.0, 1.0, 1.0}, 10, 8,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        VROLineBreakMode::Justify, VROTextClipMode::ClipToBounds);

    text->setName("Text");

    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({-5, 0, -10});

    rootNode->addChildNode(textNode);

    //boxNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));

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

static int count = 0;

void VROSample::reticleTapped(VROVector3f ray, const VRORenderContext *context) {
    if (count == 0) {
        _videoA->setVolume(0.5);
    }
    else if (count == 1) {
        _videoA->setMuted(true);
    }
    else if (count == 2) {
        _videoA->setMuted(false);
    }
    else if (count == 3) {
        _videoA->setVolume(1.0);
    }
    else if (count == 4) {
        _videoA->seekToTime(2);
    }
    else if (count == 5) {
        _videoA->pause();
    }
    else if (count == 6) {
        _videoA->play();
    }

    ++count;
    if (count == 7) {
        count = 0;
    }
}

void VROSample::onFrameWillRender(const VRORenderContext &context) {

}

void VROSample::onFrameDidRender(const VRORenderContext &context) {

}
