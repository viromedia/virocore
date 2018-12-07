//
//  VROIKTest.cpp
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTestUtil.h"
#include "VROIKTest.h"
#include "VRONode.h"
#include "VROLog.h"
#include "VRORenderer.h"
#include "VROPolyline.h"
#include "VROEventDelegate.h"
#include "VROIKRig.h"

VROIKTest::VROIKTest():VRORendererTest(VRORendererTestType::InverseKinematics) {
}

VROIKTest::~VROIKTest() {
}

void VROIKTest::build(std::shared_ptr<VRORenderer> renderer,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
    // Basic test scene setup
    _driver = driver;
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->setTag("Root Node");

    std::shared_ptr<VROTexture> environment = VROTestUtil::loadRadianceHDRTexture("ibl_wooden_door");
    rootNode->setBackgroundSphere(environment);

    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    _pointOfView = cameraNode;

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 1.0, 1.0, 1.0 });
    ambient->setIntensity(200);
    rootNode->addLight(ambient);
    _eventDelegate = std::make_shared<VROIKEventDelegate>(this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnDrag, true);

    // Create a polyline for drawing the rig skeleton
    std::shared_ptr<VROMaterial> pencilMaterial = std::make_shared<VROMaterial>();
    pencilMaterial->getDiffuse().setColor({1.0, 0, 0, 1.0});
    pencilMaterial->setCullMode(VROCullMode::None);
    pencilMaterial->setLightingModel(VROLightingModel::Constant);
    pencilMaterial->setWritesToDepthBuffer(false);
    pencilMaterial->setReadsFromDepthBuffer(false);
    std::vector<VROVector3f> pathTot;
    pathTot.push_back(VROVector3f(0,0,-100));
    pathTot.push_back(VROVector3f(0,0,-200));
    _debugRigSkeletalLine = VROPolyline::createPolyline(pathTot, 0.0025f);
    _debugRigSkeletalLine->setMaterials({ pencilMaterial });
    std::shared_ptr<VRONode> _debugRigSkeletalLineNode = std::make_shared<VRONode>();
    _debugRigSkeletalLineNode->setIgnoreEventHandling(true);
    _debugRigSkeletalLineNode->setGeometry(_debugRigSkeletalLine);
    _sceneController->getScene()->getRootNode()->addChildNode(_debugRigSkeletalLineNode);

    // Create our individual IK tests
    // testSingleVerticalChainRig();
    // testSingleHorizontalChainRig();
    // testSingleTJointRig();
    // testSingleTJointRigComplex();
    test3DSkinner(driver);

    frameSynchronizer->addFrameListener(shared_from_this());
}

void VROIKTest::testSingleVerticalChainRig() {
    _currentRoot = std::make_shared<VRONode>();
    _currentRoot->setPosition(VROVector3f(0, -0.6, -1));
    _sceneController->getScene()->getRootNode()->addChildNode(_currentRoot);

    // Create a single continous node line.
    int numOfBlocks = 5;
    std::shared_ptr<VRONode> parentNode = _currentRoot;
    for (int i = 0 ; i < numOfBlocks; i ++) {
        // Create the block.
        std::shared_ptr<VRONode> newBlockNode = createBlock(false,
                                                            "Block_" + VROStringUtil::toString(i));
        newBlockNode->setPosition(VROVector3f(0, 0.2,0));
        parentNode->addChildNode(newBlockNode);
        parentNode = newBlockNode;
    }

    // Assign the end affector node.
    std::map<std::string, std::shared_ptr<VRONode>> endEffectorNodes;
    endEffectorNodes["end"] = parentNode;

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox = createBlock(true, "Target");
    targetBox->setTag("end");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox);
    targetBox->setPosition(VROVector3f((0.2 * numOfBlocks)-0.6, 0, -1));
    _targetBoxes.push_back(targetBox);

    // Initialize the rig here.
    _rig = std::make_shared<VROIKRig>(_currentRoot, endEffectorNodes);
    _currentRoot->setIKRig(_rig);
}

void VROIKTest::testSingleHorizontalChainRig() {
    _currentRoot = std::make_shared<VRONode>();
    _currentRoot->setPosition(VROVector3f(-0.6, 0, -1));
    _sceneController->getScene()->getRootNode()->addChildNode(_currentRoot);

    // Create a single continous node line, but horizontally this time
    int numOfBlocks = 5;
    std::shared_ptr<VRONode> parentNode = _currentRoot;
    for (int i = 0 ; i < numOfBlocks; i ++) {
        // Create the block.
        std::shared_ptr<VRONode> newBlockNode = createBlock(false, "Block_" + VROStringUtil::toString(i));
        newBlockNode->setPosition(VROVector3f(0.2, 0,0));
        parentNode->addChildNode(newBlockNode);
        parentNode = newBlockNode;
    }

    // Assign the end affector node.
    std::map<std::string, std::shared_ptr<VRONode>> endEffectorNodes;
    endEffectorNodes["end"] = parentNode;

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox = createBlock(true, "Target");
    targetBox->setTag("end");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox);
    targetBox->setPosition(VROVector3f((0.2 * numOfBlocks)-0.6, 0, -1));
    _targetBoxes.push_back(targetBox);

    // Initialize the rig here.
    _rig = std::make_shared<VROIKRig>(_currentRoot, endEffectorNodes);
    _currentRoot->setIKRig(_rig);
}

void VROIKTest::testSingleTJointRig() {
    _currentRoot = std::make_shared<VRONode>();
    _currentRoot->setPosition(VROVector3f(0, -1, -1));
    _sceneController->getScene()->getRootNode()->addChildNode(_currentRoot);

    /*
     Create a IK Tree in the following format:

                                       |-> (n4) -> (End Effector)
                                       |
     (root) -> (n1) -> (n2) -> (n3) -> |-> (End Effector n5)
                                       |
                                       |-> (n6) -> (n) -> (n) -> (End Effector)
     */

    std::shared_ptr<VRONode> n1 = createBlock(false, "Block_1");
    n1->setPosition(VROVector3f(0, 0.2, 0));
    _currentRoot->addChildNode(n1);

    std::shared_ptr<VRONode> n2 = createBlock(false, "Block_2");
    n2->setPosition(VROVector3f(0, 0.2, 0));
    n1->addChildNode(n2);

    std::shared_ptr<VRONode> n3 = createBlock(false, "Block_3");
    n3->setPosition(VROVector3f(0, 0.2, 0));
    n2->addChildNode(n3);

    // Now branch out from n3
    std::shared_ptr<VRONode> n4 = createBlock(false, "Block_4");
    n4->setPosition(VROVector3f(0.2, 0.2, 0));
    n3->addChildNode(n4);

    std::shared_ptr<VRONode> n5 = createBlock(false, "Block_5");
    n5->setPosition(VROVector3f(0, 0.2, 0));
    n3->addChildNode(n5);

    std::shared_ptr<VRONode> n6 = createBlock(false, "Block_6");
    n6->setPosition(VROVector3f(-0.2, 0.2, 0));
    n3->addChildNode(n6);

    // Add End Effector to n4
    std::shared_ptr<VRONode> n4A = createBlock(false, "Block_7");
    n4A->setPosition(VROVector3f(0, 0.2, 0));
    n4->addChildNode(n4A);

    // Add End Effector branch to N6
    std::shared_ptr<VRONode> n6A = createBlock(false, "Block_8");
    n6A->setPosition(VROVector3f(0, 0.2, 0));
    n6->addChildNode(n6A);

    std::shared_ptr<VRONode> n6B = createBlock(false, "Block_9");
    n6B->setPosition(VROVector3f(0, 0.2, 0));
    n6A->addChildNode(n6B);

    std::shared_ptr<VRONode> n6C = createBlock(false, "Block_10");
    n6C->setPosition(VROVector3f(0, 0.2, 0));
    n6B->addChildNode(n6C);

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox = createBlock(true, "Target1");
    targetBox->setTag("endN4");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox);
    targetBox->setPosition(VROVector3f(0.2, 0, -1));
    _targetBoxes.push_back(targetBox);

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox2 = createBlock(true, "Target1");
    targetBox2->setTag("endN5");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox2);
    targetBox2->setPosition(VROVector3f(0, -0.2 , -1));
    _targetBoxes.push_back(targetBox2);

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox3 = createBlock(true, "Target1");
    targetBox3->setTag("endN6");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox3);
    targetBox3->setPosition(VROVector3f(-0.2, 0.4, -1));
    _targetBoxes.push_back(targetBox3);

    // Assign the end affector node.
    std::map<std::string, std::shared_ptr<VRONode>> endEffectorNodes;
    endEffectorNodes["endN4"] = n4A;
    endEffectorNodes["endN5"] = n5;
    endEffectorNodes["endN6"] = n6C;

    // Initialize the rig here.
    _rig = std::make_shared<VROIKRig>(_currentRoot, endEffectorNodes);
    _currentRoot->setIKRig(_rig);
}

void VROIKTest::testSingleTJointRigComplex() {
    /*
     Create an IK Rig to test as so:
         (n8) <-|                                   |-> (n4) -> (End Effector)
                |                                   |
         (n7) <-  (root) -> (n1) -> (n2) -> (n3) -> |-> (End Effector n5)
                |                                   |
(n10) <- (n9) <-|                                   |-> (n6) -> (n) -> (n) -> (End Effector)
 */

    _currentRoot = std::make_shared<VRONode>();
    _currentRoot->setPosition(VROVector3f(0, -1, -1));
    _sceneController->getScene()->getRootNode()->addChildNode(_currentRoot);

    std::shared_ptr<VRONode> n8 = createBlock(false, "Block_8");
    n8->setPosition(VROVector3f(0.2, -0.2, 0));
    _currentRoot->addChildNode(n8);


    std::shared_ptr<VRONode> n7 = createBlock(false, "Block_7");
    n7->setPosition(VROVector3f(0, -0.2, 0));
    _currentRoot->addChildNode(n7);


    std::shared_ptr<VRONode> n9 = createBlock(false, "Block_9");
    n9->setPosition(VROVector3f(-0.2, -0.2, 0));
    _currentRoot->addChildNode(n9);

    std::shared_ptr<VRONode> n10 = createBlock(false, "Block_10");
    n10->setPosition(VROVector3f(0.0, -0.2, 0));
    n9->addChildNode(n10);

    std::shared_ptr<VRONode> n1 = createBlock(false, "Block_1");
    n1->setPosition(VROVector3f(0, 0.2, 0));
    _currentRoot->addChildNode(n1);

    std::shared_ptr<VRONode> n2 = createBlock(false, "Block_2");
    n2->setPosition(VROVector3f(0, 0.2, 0));
    n1->addChildNode(n2);

    std::shared_ptr<VRONode> n3 = createBlock(false, "Block_3");
    n3->setPosition(VROVector3f(0, 0.2, 0));
    n2->addChildNode(n3);

    // Now branch out from n3
    std::shared_ptr<VRONode> n4 = createBlock(false, "Block_4");
    n4->setPosition(VROVector3f(0.2, 0.2, 0));
    n3->addChildNode(n4);

    std::shared_ptr<VRONode> n5 = createBlock(false, "Block_5");
    n5->setPosition(VROVector3f(0, 0.2, 0));
    n3->addChildNode(n5);

    std::shared_ptr<VRONode> n6 = createBlock(false, "Block_6");
    n6->setPosition(VROVector3f(-0.2, 0.2, 0));
    n3->addChildNode(n6);

    // Add End Effector to n4
    std::shared_ptr<VRONode> n4A = createBlock(false, "Block_7");
    n4A->setPosition(VROVector3f(0, 0.2, 0));
    n4->addChildNode(n4A);

    // Add End Effector branch to N6
    std::shared_ptr<VRONode> n6A = createBlock(false, "Block_8");
    n6A->setPosition(VROVector3f(0, 0.2, 0));
    n6->addChildNode(n6A);

    std::shared_ptr<VRONode> n6B = createBlock(false, "Block_9");
    n6B->setPosition(VROVector3f(0, 0.2, 0));
    n6A->addChildNode(n6B);

    std::shared_ptr<VRONode> n6C = createBlock(false, "Block_10");
    n6C->setPosition(VROVector3f(0, 0.2, 0));
    n6B->addChildNode(n6C);

    // Create bottom boxes
    std::shared_ptr<VRONode> targetBoxA = createBlock(true, "Target1");
    targetBoxA->setTag("endN7");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBoxA);
    targetBoxA->setPosition(VROVector3f(0, -1.2, -1));
    _targetBoxes.push_back(targetBoxA);
    
    std::shared_ptr<VRONode> targetBoxB = createBlock(true, "Target1");
    targetBoxB->setTag("endN8");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBoxB);
    targetBoxB->setPosition(VROVector3f(0.2, -1.2, -1));
    _targetBoxes.push_back(targetBoxB);

    std::shared_ptr<VRONode> targetBoxD = createBlock(true, "Target1");
    targetBoxD->setTag("endN10");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBoxD);
    targetBoxD->setPosition(VROVector3f(-0.2, -1.4, -1));
    _targetBoxes.push_back(targetBoxD);
    
    // Create target boxes.
    std::shared_ptr<VRONode> targetBox = createBlock(true, "Target1");
    targetBox->setTag("endN4");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox);
    targetBox->setPosition(VROVector3f(0.2, 0, -1));
    _targetBoxes.push_back(targetBox);

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox2 = createBlock(true, "Target1");
    targetBox2->setTag("endN5");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox2);
    targetBox2->setPosition(VROVector3f(0, -0.2 , -1));
    _targetBoxes.push_back(targetBox2);

    // Create target boxes.
    std::shared_ptr<VRONode> targetBox3 = createBlock(true, "Target1");
    targetBox3->setTag("endN6");
    _sceneController->getScene()->getRootNode()->addChildNode(targetBox3);
    targetBox3->setPosition(VROVector3f(-0.2, 0.4, -1));
    _targetBoxes.push_back(targetBox3);


    // Assign the end affector node.
    std::map<std::string, std::shared_ptr<VRONode>> endEffectorNodes;
    endEffectorNodes["endN4"] = n4A;
    endEffectorNodes["endN5"] = n5;
    endEffectorNodes["endN6"] = n6C;

    endEffectorNodes["endN7"]  = n7;
    endEffectorNodes["endN8"]  = n8;
    endEffectorNodes["endN10"] = n10;
    
    // Initialize the rig here.
    _rig = std::make_shared<VROIKRig>(_currentRoot, endEffectorNodes);
    _currentRoot->setIKRig(_rig);
}

void VROIKTest::test3DSkinner(std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 1.0, 1.0 });
    light->setPosition( { 0, 10, 10 });
    light->setDirection( { 0, -1.0, -1.0 });
    light->setAttenuationStartDistance(25);
    light->setAttenuationEndDistance(50);
    light->setSpotInnerAngle(35);
    light->setSpotOuterAngle(60);
    light->setCastsShadow(true);
    light->setIntensity(800);
    _sceneController->getScene()->getRootNode()->addLight(light);

    _is3DModelTest = true;
    VROVector3f pos = VROVector3f( 0, -0.5, -0.6);
    VROVector3f scale = VROVector3f( 1,  1,  1 );
    std::shared_ptr<VRONode> rootgLTFNode =  VROTestUtil::loadGLTFModel("CesiumMan","glb",
                                                                        pos, scale, 1, "", driver,
                                                                        [this](std::shared_ptr<VRONode> node, bool success){
                                                                            node->setIgnoreEventHandling(true);
                                                                            node->setScale(VROVector3f(0.5,0.5,0.5));
                                                                            node->setRotationEuler(VROVector3f(0,toRadians(-90),0));
                                                                            initSkinner(node);
                                                                        });
    _sceneController->getScene()->getRootNode()->addChildNode(rootgLTFNode);

    // Reset Box.
    std::shared_ptr<VROBox> debugResetBox = VROBox::createBox(0.06, 0.06, 0.06);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setCullMode(VROCullMode::None);
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    mat->getDiffuse().setColor(VROVector4f(0.0, 1.0, 0, 1.0));
    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    debugResetBox->setMaterials(mats);

    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(debugResetBox);
    debugNode->setRenderingOrder(10);
    debugNode->setTag("RESET");
    debugNode->setEventDelegate(_eventDelegate);
    debugNode->setPosition(VROVector3f(-1,-1,-1));
    _sceneController->getScene()->getRootNode()->addChildNode(debugNode);
}

void VROIKTest::initSkinner(std::shared_ptr<VRONode> gltfNode) {
    std::vector<std::shared_ptr<VROSkinner>> skinners;
    gltfNode->getSkinner(skinners, true);
    if (skinners.size() < 0) {
        return;
    }
    _skinner = skinners[0];
    _endEffectorBones["leftHand"] = 9;
    _endEffectorBones["rightHand"] = 10;
    _endEffectorBones["leftFeet"] = 16;
    _endEffectorBones["rightFeet"] = 15;
    _endEffectorBones["head"] = 4;

    if (!_initWithRig) {
        return;
    }
    _rig = std::make_shared<VROIKRig>(skinners[0], _endEffectorBones);
    gltfNode->setIKRig(_rig);
}

void VROIKTest::calculateSkeletalLines(std::shared_ptr<VRONode> node, std::vector<std::vector<VROVector3f>> &paths) {
    if (node->getChildNodes().size() <=0) {
        return;
    }

    VROVector3f parentLoc = node->getWorldPosition();
    for (auto child : node->getChildNodes()) {
        VROVector3f nodeLoc = child->getWorldPosition();

        std::vector<VROVector3f> pathTot;
        pathTot.push_back(parentLoc);
        pathTot.push_back(nodeLoc);
        paths.push_back(pathTot);
    }

    for (auto child : node->getChildNodes()) {
        calculateSkeletalLines(child, paths);
    }
}

void VROIKTest::refreshBasicNonSkeletalRig() {
    std::vector<std::vector<VROVector3f>> paths;
    calculateSkeletalLines(_currentRoot, paths);
    _debugRigSkeletalLine->setPaths(paths);
}

void VROIKTest::refreshSkeletalRig() {
    for (auto ef : _endEffectorBones) {
        VROVector3f pos = _skinner->getCurrentBoneWorldTransform(ef.second).extractTranslation();
        std::shared_ptr<VRONode> block = createGLTFEffectorBlock(true, ef.first);
        if (_rig == nullptr) {
            block->setTag("Bone:" + VROStringUtil::toString(ef.second));
        }

        _sceneController->getScene()->getRootNode()->addChildNode(block);
        VROMatrix4f ident;
        ident.toIdentity();
        block->setWorldTransform(pos, ident);
    }
}

std::shared_ptr<VRONode> VROIKTest::createBlock(bool isAffector, std::string tag) {
    // Create our debug box node
    float dimen = 0.03;
    if (isAffector) {
        dimen = 0.035;
    }
    std::shared_ptr<VROBox> box = VROBox::createBox(dimen*4, dimen, dimen);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setCullMode(VROCullMode::None);
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    if (isAffector) {
        mat->getDiffuse().setColor(VROVector4f(0.0, 1.0, 0, 1.0));
    } else {
        mat->getDiffuse().setColor(VROVector4f(1.0, 0, 0, 1.0));
    }

    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    box->setMaterials(mats);

    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(box);
    debugNode->setRenderingOrder(20);
    debugNode->setTag(tag);
    if (isAffector) {
        debugNode->setEventDelegate(_eventDelegate);
        debugNode->setDragType(VRODragType::FixedToPlane);
        debugNode->setDragPlanePoint({0,0,-1});
        debugNode->setDragPlaneNormal({0,0,1});
    } else {
        debugNode->setIgnoreEventHandling(true);
    }

    return debugNode;
}

std::shared_ptr<VRONode> VROIKTest::createGLTFEffectorBlock(bool isAffector, std::string tag) {
    // Create our debug box node
    float dimen = 0.03;
    if (isAffector) {
        dimen =  0.03;
    }
    std::shared_ptr<VROBox> box = VROBox::createBox(dimen, dimen, dimen);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setCullMode(VROCullMode::None);
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    if (isAffector) {
        mat->getDiffuse().setColor(VROVector4f(0.0, 1.0, 0, 1.0));
    } else {
        mat->getDiffuse().setColor(VROVector4f(1.0, 0, 0, 1.0));
    }

    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    box->setMaterials(mats);

    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(box);
    debugNode->setRenderingOrder(10);
    debugNode->setTag(tag);
    if (isAffector) {
        debugNode->setEventDelegate(_eventDelegate);
        debugNode->setDragType(VRODragType::FixedDistance);
    } else {
        debugNode->setIgnoreEventHandling(true);
    }

    return debugNode;
}


void VROIKTest::onFrameWillRender(const VRORenderContext &context) {
    if (!_is3DModelTest) {
        refreshBasicNonSkeletalRig();
    }
}

void VROIKTest::onFrameDidRender(const VRORenderContext &context) {
}

void VROIKTest::onClick(int source,
                        std::shared_ptr<VRONode> node,
                        VROEventDelegate::ClickState clickState,
                        std::vector<float> position) {
    if (!_is3DModelTest) {
        return;
    }

    if (clickState != VROEventDelegate::ClickState::ClickUp) {
        return;
    }

    std::string tag = node->getTag();
    if (VROStringUtil::strcmpinsensitive(node->getTag(), "RESET")) {pwarn("Daniel 1");
        refreshSkeletalRig();
    }
}

void VROIKTest::onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition) {
    if (!_is3DModelTest) {
        _rig->setPositionForEffector(node->getTag(), newPosition);
        return;
    }

    std::string tag = node->getTag();
    if (VROStringUtil::startsWith(tag, "Bone:")) {
        std::string boneTag = tag.substr(5, tag.size() - 5);
        int boneId = VROStringUtil::toInt(boneTag);
        VROMatrix4f s = _skinner->getCurrentBoneWorldTransform(boneId);
        VROMatrix4f f = VROMatrix4f::identity();
        f.rotate(s.extractRotation(s.extractScale()));
        f.translate(node->getWorldTransform().extractTranslation());
        _skinner->setCurrentBoneWorldTransform(boneId, f);
    } else if (_rig != nullptr) {
        _rig->setPositionForEffector(tag, node->getWorldPosition());
    }
}