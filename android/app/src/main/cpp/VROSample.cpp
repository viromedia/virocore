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
#include "VROVideoTextureAVP.h"
#include "VROText.h"
#include "VROPlatformUtil.h"
#include "VROOBJLoader.h"

VROSample::VROSample() {

}

VROSample::~VROSample() {

}

std::shared_ptr<VROSceneController> VROSample::loadBoxScene(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                            std::shared_ptr<VRODriver> driver) {


    _driver = driver;
    frameSynchronizer->addFrameListener(shared_from_this());

    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube(getNiagaraTexture());

    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.3, 0.3, 0.3 });

    std::shared_ptr<VROLight> spot = std::make_shared<VROLight>(VROLightType::Spot);
    spot->setColor({ 1.0, 1.0, 1.0 });
    spot->setPosition( { 0, 0, 0 });
    spot->setDirection( { 0, 0, -1 });
    spot->setAttenuationStartDistance(5);
    spot->setAttenuationEndDistance(10);
    spot->setSpotInnerAngle(10.0);
    spot->setSpotOuterAngle(20.0);

    rootNode->addLight(ambient);
    rootNode->addLight(spot);

    scene->addNode(rootNode);

    VROTextureInternalFormat format = VROTextureInternalFormat::RGB565;

    /*
     Create the obj node.
     */
    std::string heartPath = VROPlatformCopyAssetToFile("heart.obj");
    std::shared_ptr<VRONode> heartNode = VROOBJLoader::loadOBJFromFile(heartPath, "", true, [format](std::shared_ptr<VRONode> node, bool success) {
        if (!success) {
            return;
        }

        std::shared_ptr<VROMaterial> &material = node->getGeometry()->getMaterials().front();
        material->getDiffuse().setTexture(std::make_shared<VROTexture>(format,
                                                                       VROMipmapMode::Runtime,
                                                                       VROPlatformLoadImageFromAsset("Heart_D4.jpg", format)));
        material->getSpecular().setTexture(std::make_shared<VROTexture>(format,
                                                                       VROMipmapMode::Runtime,
                                                                       VROPlatformLoadImageFromAsset("Heart_S2.jpg", format)));
        material->setLightingModel(VROLightingModel::Phong);
    });

    heartNode->setPosition({0, -5.25, -1});
    _objAngle = 0;
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this](VRONode *const node, float seconds) {
        _objAngle += .01;
        node->setRotation({ 0, _objAngle, 0});

        return true;
    });
    heartNode->runAction(action);
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
    maleNode->setPosition({-10, -100, -10});
    maleNode->setScale({ 0.1, 0.1, 0.1 });
    rootNode->addChildNode(maleNode);

    std::shared_ptr<VROAction> maleAction = VROAction::perpetualPerFrameAction([this](VRONode *const node, float seconds) {
        node->setRotation({ 0, _objAngle, 0});
        return true;
    });
    maleNode->runAction(maleAction);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");

    std::string videoPath = VROPlatformCopyAssetToFile("vest.mp4");

    _material = box->getMaterials()[0];
    _material->setLightingModel(VROLightingModel::Lambert);
    //_material->getDiffuse().setTexture(_videoA);
    _material->getDiffuse().setTexture(std::make_shared<VROTexture>(format,
                                                                    VROMipmapMode::Runtime,
                                                                    VROPlatformLoadImageFromAsset("boba.png", format)));
    _material->getSpecular().setTexture(std::make_shared<VROTexture>(format,
                                                                     VROMipmapMode::Runtime,
                                                                     VROPlatformLoadImageFromAsset("specular.png", format)));

    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -15});

    rootNode->addChildNode(boxNode);

    std::string texFile = VROPlatformCopyAssetToFile("card_main.ktx");
    int texLength;
    void *texBytes = VROPlatformLoadFile(texFile, &texLength);

    VROTextureFormat texFormat;
    int texWidth;
    int texHeight;
    std::vector<uint32_t> mipSizes;
    std::shared_ptr<VROData> texData = VROTextureUtil::readKTXHeader((uint8_t *)texBytes, (uint32_t)texLength,
                                                                     &texFormat, &texWidth, &texHeight, &mipSizes);
    std::vector<std::shared_ptr<VROData>> dataVec = { texData };

    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, texFormat,
                                                                       VROTextureInternalFormat::RGBA8,
                                                                       VROMipmapMode::Pregenerated,
                                                                       dataVec, texWidth, texHeight, mipSizes,
                                                                       driver.get());

    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(10, 10, 0, 0, 1, 1);
    surface->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    surface->getMaterials().front()->getDiffuse().setTexture(texture);

    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setPosition({0, 0, -5});

    rootNode->addChildNode(surfaceNode);

    std::string string = "In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    std::shared_ptr<VROTypeface> typeface = driver->newTypeface("Roboto", 8);
    std::shared_ptr<VROText> text = VROText::createText(string, typeface, {1.0, 1.0, 1.0, 1.0}, 10, 8,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        VROLineBreakMode::Justify, VROTextClipMode::ClipToBounds);

    text->setName("Text");

    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({10, 0, -10});

    //rootNode->addChildNode(textNode);

    return sceneController;
}

std::shared_ptr<VROTexture> VROSample::getNiagaraTexture() {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGB565;
    std::vector<std::shared_ptr<VROImage>> cubeImages = {
            std::make_shared<VROImageAndroid>("px.png", format),
            std::make_shared<VROImageAndroid>("nx.png", format),
            std::make_shared<VROImageAndroid>("py.png", format),
            std::make_shared<VROImageAndroid>("ny.png", format),
            std::make_shared<VROImageAndroid>("pz.png", format),
            std::make_shared<VROImageAndroid>("nz.png", format)
    };

    return std::make_shared<VROTexture>(format, cubeImages);
}

void VROSample::reticleTapped(VROVector3f ray, const VRORenderContext *context) {

}

void VROSample::onFrameWillRender(const VRORenderContext &context) {

}

void VROSample::onFrameDidRender(const VRORenderContext &context) {

}
