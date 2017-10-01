//
//  VROParticleTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROParticleTest.h"
#include "VROTestUtil.h"

VROParticleTest::VROParticleTest() :
    VRORendererTest(VRORendererTestType::Particle) {
        
}

VROParticleTest::~VROParticleTest() {
    
}

void VROParticleTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                            std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    scene->getRootNode()->addChildNode(rootNode);
    
    std::shared_ptr<VRONode> particleNode = std::make_shared<VRONode>();
    particleNode->setPosition({0, -10, -15});
    particleNode->setTag("Particles");
    
    std::shared_ptr<VROTexture> imgTexture = VROTestUtil::loadDiffuseTexture("cloud");
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1,1);
    std::shared_ptr<VROParticleEmitter> particleEmitter = std::make_shared<VROParticleEmitter>(driver,
                                                                                               particleNode,
                                                                                               surface);
    // Vec of intervals to interpolate this modifier along.
    std::vector<VROParticleModifier::VROModifierInterval> intervals;
    VROParticleModifier::VROModifierInterval interval1;
    interval1.startFactor = 0;
    interval1.endFactor = 1000;
    interval1.targetedValue = VROVector3f(0,0,0);
    intervals.push_back(interval1);
    
    // Modifier's starting configuration. Provide different numbers to randomize.
    VROVector3f sizeMinStart = VROVector3f(2,2,2);
    VROVector3f sizeMaxStart = VROVector3f(2,2,2);
    
    std::shared_ptr<VROParticleModifier> mod = std::make_shared<VROParticleModifier>(sizeMinStart,
                                                                                     sizeMaxStart,
                                                                                     VROParticleModifier::VROModifierFactor::Time,
                                                                                     intervals);
    // Finally set this modifier.
    particleEmitter->setScaleModifier(mod);
    
    scene->addParticleEmitter(particleEmitter);
    rootNode->addChildNode(particleNode);
    
    /*
     // Uncomment to test animating a particle emitter.
     dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
     VROTransaction::begin();
     VROTransaction::setAnimationDuration(64);
     particleNode->setRotationEulerY(96);
     VROTransaction::commit();
     });
     */
}
