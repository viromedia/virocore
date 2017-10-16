//
//  VRORendererTestHarness.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRORendererTestHarness.h"
#include "VRORendererTest.h"
#include "VROBoxTest.h"
#include "VROBloomTest.h"
#include "VROARDraggableNodeTest.h"
#include "VROARPlaneTest.h"
#include "VROARShadowTest.h"
#include "VROParticleTest.h"
#include "VROFBXTest.h"
#include "VROHDRTest.h"
#include "VRONormalMapTest.h"
#include "VROOBJTest.h"
#include "VROPerfTest.h"
#include "VROPortalTest.h"
#include "VROShadowTest.h"
#include "VROStereoscopicTest.h"
#include "VROTextTest.h"
#include "VROTorusTest.h"
#include "VROPhysicsTest.h"
#include "VROPolylineTest.h"
#include "VROVideoSphereTest.h"

VRORendererTestHarness::VRORendererTestHarness(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                               std::shared_ptr<VRODriver> driver) :
    _frameSynchronizer(frameSynchronizer),
    _driver(driver) {
    
}

VRORendererTestHarness::~VRORendererTestHarness() {
    
}

std::shared_ptr<VRORendererTest> VRORendererTestHarness::loadNextTest() {
    if (_currentTest == nullptr) {
        return loadTest((VRORendererTestType) 0);
    }
    else if ((int)_currentTest->getType() == (int) VRORendererTestType::NumTests - 1) {
        return loadTest((VRORendererTestType) 0);
    }
    else {
        return loadTest((VRORendererTestType) ((int) _currentTest->getType() + 1));
    }
}

std::shared_ptr<VRORendererTest> VRORendererTestHarness::loadTest(VRORendererTestType type) {
    _currentTest = createTest(type);
    _currentTest->build(_frameSynchronizer, _driver);
    return _currentTest;
}

std::shared_ptr<VRORendererTest> VRORendererTestHarness::getCurrentTest() {
    return _currentTest;
}

std::shared_ptr<VRORendererTest> VRORendererTestHarness::createTest(VRORendererTestType type) {
    switch (type) {
        case VRORendererTestType::Box:
            return std::make_shared<VROBoxTest>();
        case VRORendererTestType::OBJ:
            return std::make_shared<VROOBJTest>();
        case VRORendererTestType::Torus:
            return std::make_shared<VROTorusTest>();
        case VRORendererTestType::Particle:
            return std::make_shared<VROParticleTest>();
        case VRORendererTestType::Physics:
            return std::make_shared<VROPhysicsTest>();
        case VRORendererTestType::Text:
            return std::make_shared<VROTextTest>();
        case VRORendererTestType::VideoSphere:
            return std::make_shared<VROVideoSphereTest>();
        case VRORendererTestType::NormalMap:
            return std::make_shared<VRONormalMapTest>();
        case VRORendererTestType::Stereoscopic:
            return std::make_shared<VROStereoscopicTest>();
        case VRORendererTestType::FBX:
            return std::make_shared<VROFBXTest>();
        case VRORendererTestType::ARPlane:
            return std::make_shared<VROARPlaneTest>();
        case VRORendererTestType::ARDraggableNode:
            return std::make_shared<VROARDraggableNodeTest>();
        case VRORendererTestType::Portal:
            return std::make_shared<VROPortalTest>();
        case VRORendererTestType::Shadow:
            return std::make_shared<VROShadowTest>();
        case VRORendererTestType::ARShadow:
            return std::make_shared<VROARShadowTest>();
        case VRORendererTestType::HDR:
            return std::make_shared<VROHDRTest>();
        case VRORendererTestType::Bloom:
            return std::make_shared<VROBloomTest>();
        case VRORendererTestType::Perf:
            return std::make_shared<VROPerfTest>();
        case VRORendererTestType::Polyline:
            return std::make_shared<VROPolylineTest>();
        default:
            pabort();
            return nullptr;
    }
}

