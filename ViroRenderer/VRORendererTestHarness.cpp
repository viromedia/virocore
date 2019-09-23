//
//  VRORendererTestHarness.cpp
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

#include "VRORendererTestHarness.h"
#include "VRORendererTest.h"
#include "VROBoxTest.h"
#include "VROBloomTest.h"
#include "VROARDraggableNodeTest.h"
#include "VROARPlaneTest.h"
#include "VROARShadowTest.h"
#include "VROParticleTest.h"
#include "VROFBXTest.h"
#include "VROGLTFTest.h"
#include "VROIKTest.h"
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
#include "VROPBRDirectTest.h"
#include "VROPBRTexturedTest.h"
#include "VRODiffuseIrradianceTest.h"
#include "VROPhotometricLightTest.h"
#include "VROVideoSphereTest.h"
#include "VRORendererSettingsTest.h"
#include "VROToneMappingTest.h"
#include "VROPolygonTest.h"
#include "VROBodyTrackerTest.h"
#include "VROObjectRecognitionTest.h"
#include "VROBodyRecognitionTest.h"
#include "VROBodyMesherTest.h"

#if VRO_PLATFORM_IOS
#include "VROARImageTrackingTest.h"
#include "VROARObjectTrackingTest.h"
#endif

VRORendererTestHarness::VRORendererTestHarness(std::shared_ptr<VRORenderer> renderer,
                                               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                               std::shared_ptr<VRODriver> driver) :
    _renderer(renderer),
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
    _currentTest->build(_renderer, _frameSynchronizer, _driver);
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
        case VRORendererTestType::GLTF:
            return std::make_shared<VROGLTFTest>();
        case VRORendererTestType::InverseKinematics:
            return std::make_shared<VROIKTest>();
        case VRORendererTestType::ARPlane:
            return std::make_shared<VROARPlaneTest>();
        case VRORendererTestType::ARDraggableNode:
            return std::make_shared<VROARDraggableNodeTest>();
#if VRO_PLATFORM_IOS
        case VRORendererTestType::ARImageTracking:
            return std::make_shared<VROARImageTrackingTest>();
        case VRORendererTestType::ARObjectTracking:
            return std::make_shared<VROARObjectTrackingTest>();
#endif
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
        case VRORendererTestType::PBRDirect:
            return std::make_shared<VROPBRDirectTest>();
        case VRORendererTestType::PBRTextured:
            return std::make_shared<VROPBRTexturedTest>();
        case VRORendererTestType::DiffuseIrradiance:
            return std::make_shared<VRODiffuseIrradianceTest>();
        case VRORendererTestType::PhotometricLight:
            return std::make_shared<VROPhotometricLightTest>();
        case VRORendererTestType::RendererSettings:
            return std::make_shared<VRORendererSettingsTest>();
        case VRORendererTestType::ToneMapping:
            return std::make_shared<VROToneMappingTest>();
        case VRORendererTestType::Polygon:
            return std::make_shared<VROPolygonTest>();
        case VRORendererTestType::BodyTracker:
            return std::make_shared<VROBodyTrackerTest>();
        case VRORendererTestType::ObjectRecognition:
            return std::make_shared<VROObjectRecognitionTest>();
        case VRORendererTestType::BodyRecognition:
            return std::make_shared<VROBodyRecognitionTest>();
        case VRORendererTestType::BodyMesher:
            return std::make_shared<VROBodyMesherTest>();
        default:
            pabort();
            return nullptr;
    }
}

