//
//  VROPortalTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortalTest.h"
#include "VROTestUtil.h"
#include "VROPortalFrame.h"
#include "VROModelIOUtil.h"

VROPortalTest::VROPortalTest() :
    VRORendererTest(VRORendererTestType::Portal) {
        
}

VROPortalTest::~VROPortalTest() {
    
}

void VROPortalTest::build(std::shared_ptr<VRORenderer> renderer,
                          std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                          std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.5, 0.5 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(70);
    light->setSpotOuterAngle(120);
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 1.0, 1.0, 1.0 });
    ambient->setIntensity(250);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    rootNode->addLight(ambient);
    
    std::shared_ptr<VROPortal> portalNode = std::make_shared<VROPortal>();
    portalNode->setBackgroundCube(VROTestUtil::loadCloudBackground());
    portalNode->addChildNode(VROTestUtil::loadFBXModel("aliengirl", { 0, -3, -6 }, { 0.04, 0.04, 0.04 }, 1, "Take 001", driver));
    portalNode->setPassable(true);
    portalNode->setScale({0.1, 0.1, 0.1});
    portalNode->setPosition({0, 0, -2});
    portalNode->setName("Portal");
    
    std::shared_ptr<VROPortalFrame> portalNodeEntrance = loadPortalEntrance(driver);
    portalNodeEntrance->setOpacity(0);
    portalNodeEntrance->setScale({0, 0, 0});
    portalNodeEntrance->setRotationEuler({ 0, 0, 0 });
    portalNode->setPortalEntrance(portalNodeEntrance);
    
    rootNode->addChildNode(portalNode);
    
    std::shared_ptr<VRONode> occludingInnerSurface = std::make_shared<VRONode>();
    occludingInnerSurface->setGeometry(VROSurface::createSurface(15, 15));
    occludingInnerSurface->setPosition({0, 0, -1});
    
    // Uncomment to test a large surface inside the portal; it should clip against the
    // portal edges
    //portalNode->addChildNode(occludingInnerSurface);
    
    std::shared_ptr<VRONode> occludingBox = std::make_shared<VRONode>();
    occludingBox->setGeometry(VROBox::createBox(0.3, 0.3, 0.3));
    occludingBox->setPosition({0.2, 0, -1});
    occludingBox->getGeometry()->getMaterials().front()->setLightingModel(VROLightingModel::Lambert);
    occludingBox->getGeometry()->getMaterials().front()->getDiffuse().setColor({1.0, 0.0, 0.0, 1.0});
    
    //rootNode->addChildNode(occludingBox);
    
    bool testFbxPortals = true;
    
    if (!testFbxPortals) {
        std::vector<VROVector3f> positions;
        positions.push_back({0, 6, -8});
        positions.push_back({0, -3, -2});
        positions.push_back({0, -8, -2});
        positions.push_back({-5, 0, -2});
        
        for (VROVector3f position : positions) {
            std::shared_ptr<VROPortal> innerPortalNode = std::make_shared<VROPortal>();
            innerPortalNode->setPosition(position);
            innerPortalNode->setBackgroundSphere(VROTestUtil::loadWestlakeBackground());
            
            std::shared_ptr<VROPortalFrame> innerPortalFrame = loadPortalEntrance(driver);
            innerPortalFrame->setScale({0.25, 0.25, 0.25});
            innerPortalNode->setPortalEntrance(innerPortalFrame);
            
            std::shared_ptr<VRONode> innerPortalNodeContent = std::make_shared<VRONode>();
            innerPortalNodeContent->setGeometry(VROBox::createBox(0.5, 0.5, 0.5));
            innerPortalNodeContent->getGeometry()->getMaterials().front()->getDiffuse().setColor({0.0, 0.0, 1.0, 1.0});
            innerPortalNodeContent->setPosition({0.2, 0, -1});
            innerPortalNode->addChildNode(innerPortalNodeContent);
            //innerPortalNode->setPassable(true);
            
            std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([] (VRONode *const node, float seconds) {
                node->setRotation({ 0, (float)(node->getRotationEuler().y + 0.15), 0});
                return true;
            });
            innerPortalNodeContent->runAction(action);
            
            portalNode->addChildNode(innerPortalNode);
        }
    } else {
        std::vector<VROVector3f> positions;
        positions.push_back({2, 0, -2});
        positions.push_back({0, -3, -2});
        positions.push_back({-2, 0, -2});
        positions.push_back({0, 2, -2});
        
        std::vector<std::string> fbxPortals;
        fbxPortals.push_back("portal_window_frame");
        fbxPortals.push_back("portal_ship");
        fbxPortals.push_back("portal_archway");
        fbxPortals.push_back("portal_wood_frame");
        
        for (int i = 0; i < positions.size(); i++) {
            VROVector3f position = positions[i];
            std::shared_ptr<VROPortal> innerPortalNode = std::make_shared<VROPortal>();
            innerPortalNode->setPosition(position);
            innerPortalNode->setBackgroundSphere(VROTestUtil::loadWestlakeBackground());
            
            std::shared_ptr<VROPortalFrame> innerPortalFrame = loadFBXPortalEntrance(fbxPortals[i], 0.5, driver);
            innerPortalNode->setPortalEntrance(innerPortalFrame);
            
            std::shared_ptr<VRONode> innerPortalNodeContent = std::make_shared<VRONode>();
            innerPortalNodeContent->setGeometry(VROBox::createBox(0.5, 0.5, 0.5));
            innerPortalNodeContent->getGeometry()->getMaterials().front()->getDiffuse().setColor({0.0, 0.0, 1.0, 1.0});
            innerPortalNodeContent->setPosition({0.2, 0, -1});
            innerPortalNode->addChildNode(innerPortalNodeContent);
            //innerPortalNode->setPassable(true);
            
            std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([] (VRONode *const node, float seconds) {
                node->setRotation({ 0, (float)(node->getRotationEuler().y + 0.15), 0});
                return true;
            });
            innerPortalNodeContent->runAction(action);
            
            rootNode->addChildNode(innerPortalNode);
        }
    }
    
    std::vector<VROVector3f> sidePositions;
    sidePositions.push_back({-0.5, -1, -2.2});
    sidePositions.push_back({-0.5, 1, -4});
    
    for (VROVector3f sidePosition : sidePositions) {
        std::shared_ptr<VROPortal> sidePortalNode = std::make_shared<VROPortal>();
        sidePortalNode->setPosition(sidePosition);
        sidePortalNode->setBackgroundCube(VROTestUtil::loadNiagaraBackground());
        
        std::shared_ptr<VROPortalFrame> sidePortalFrame = loadPortalEntrance(driver);
        sidePortalFrame->setScale({0.06, 0.06, 0.06});
        sidePortalFrame->setRotationEuler({0, M_PI_4, 0});
        sidePortalNode->setPortalEntrance(sidePortalFrame);
        
        std::shared_ptr<VRONode> sidePortalNodeContent = std::make_shared<VRONode>();
        sidePortalNodeContent->setGeometry(VROBox::createBox(0.6, 0.6, 0.6));
        sidePortalNodeContent->setPosition({0.2, 0, -1});
        sidePortalNode->addChildNode(sidePortalNodeContent);
        
        rootNode->addChildNode(sidePortalNode);
    }
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(1.0);
    VROTransaction::setAnimationDuration(2.5);
    VROTransaction::setTimingFunction(VROTimingFunctionType::Bounce);
    
    portalNodeEntrance->setScale({1, 1, 1});
    portalNodeEntrance->setOpacity(1);
    
    VROTransaction::commit();
 
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    scene->getRootNode()->setCamera(camera);
    
    _pointOfView = scene->getRootNode();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(10);
    camera->setPosition({0, 0, -5});
    VROTransaction::commit();
}

std::shared_ptr<VROPortalFrame> VROPortalTest::loadPortalEntrance(std::shared_ptr<VRODriver> driver) {
    std::string url = VROTestUtil::getURLForResource("portal_ring", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VROPortalFrame> frame = std::make_shared<VROPortalFrame>();
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    VROOBJLoader::loadOBJFromResource(url, VROResourceType::URL, node, driver,
                                                                 [](std::shared_ptr<VRONode> node, bool success) {
                                                                     if (!success) {
                                                                         return;
                                                                     }
                                                                     
                                                                     std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();                                                                     
                                                                     material->setLightingModel(VROLightingModel::Lambert);
                                                                     material->getDiffuse().setTexture(VROTestUtil::loadDiffuseTexture("portal_ring"));
                                                                 });
    
    frame->addChildNode(node);
    return frame;
}

std::shared_ptr<VROPortalFrame> VROPortalTest::loadFBXPortalEntrance(std::string fbxResource, float scale,
                                                                     std::shared_ptr<VRODriver> driver) {
    std::string url = VROTestUtil::getURLForResource(fbxResource, "vrx");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VROPortalFrame> frame = std::make_shared<VROPortalFrame>();
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    VROFBXLoader::loadFBXFromResource(url, VROResourceType::URL, node, driver,
                                                                 [scale](std::shared_ptr<VRONode> node, bool success) {
                                                                     if (!success) {
                                                                         return;
                                                                     }
                                                                     
                                                                     node->setScale({scale, scale, scale});
                                                                     node->setPosition({0,0,0});
                                                                 });
    frame->addChildNode(node);
    return frame;
}
