//
//  VROParticleTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROParticleTest.h"
#include "VROTestUtil.h"
#include "VROParticleEmitter.h"

VROParticleTest::VROParticleTest() :
    VRORendererTest(VRORendererTestType::Particle) {
        
}

VROParticleTest::~VROParticleTest() {
    
}

void VROParticleTest::build(std::shared_ptr<VRORenderer> renderer,
                            std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                            std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    
    std::shared_ptr<VROTexture> environment = VROTestUtil::loadRadianceHDRTexture("ibl_mans_outside");
    rootNode->setBackgroundSphere(environment);
    
    std::shared_ptr<VRONode> particleNode = std::make_shared<VRONode>();
    particleNode->setPosition({0, -0.5, -1});
    particleNode->setTag("Particles");
    
    // Particle surface
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1, 1);
    std::shared_ptr<VROTexture> texture = VROTestUtil::loadDiffuseTexture("darkSmoke");
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setTexture(texture);
    surface->setMaterials({ material });
    
    // Core emitter settings
    std::shared_ptr<VROParticleEmitter> emitter = std::make_shared<VROParticleEmitter>(driver, surface);
    emitter->setParticleLifeTime({ 6000, 7000 });
    emitter->setEmissionRatePerSecond({ 12, 18 });
    emitter->setMaxParticles(100);
    emitter->setDuration(1000);
    emitter->setLoop(true);
    emitter->setBlendMode(VROBlendMode::Add);
    
    // Spawn volume
    VROParticleSpawnVolume volume;
    volume.shape = VROParticleSpawnVolume::Shape::Sphere;
    volume.shapeParams = { 0.1 };
    emitter->setParticleSpawnVolume(volume);
    
    // Scale modifier
    std::vector<VROParticleModifier::VROModifierInterval> scaleIntervals;
    VROParticleModifier::VROModifierInterval scaleInterval;
    scaleInterval.startFactor = 0;
    scaleInterval.endFactor = 1000;
    scaleInterval.targetedValue = VROVector3f(1, 1, 1);
    scaleIntervals.push_back(scaleInterval);
    
    VROVector3f scaleStartMin = { 0.3, 0.3, 0.3 };
    VROVector3f scaleStartMax = { 0.3, 0.3, 0.3 };
    std::shared_ptr<VROParticleModifier> scale = std::make_shared<VROParticleModifier>(scaleStartMin,
                                                                                       scaleStartMax,
                                                                                       VROParticleModifier::VROModifierFactor::Time,
                                                                                       scaleIntervals);
    emitter->setScaleModifier(scale);
    
    // Opacity modifier
    std::vector<VROParticleModifier::VROModifierInterval> opacityIntervals;
    VROParticleModifier::VROModifierInterval opacityIntervalA;
    opacityIntervalA.startFactor = 0;
    opacityIntervalA.endFactor = 1000;
    opacityIntervalA.targetedValue = VROVector3f(1, 1, 1);
    opacityIntervals.push_back(opacityIntervalA);
    
    VROParticleModifier::VROModifierInterval opacityIntervalB;
    opacityIntervalB.startFactor = 2000;
    opacityIntervalB.endFactor = 3000;
    opacityIntervalB.targetedValue = VROVector3f(0, 0, 0);
    opacityIntervals.push_back(opacityIntervalB);
    
    VROVector3f opacityStartMin = { 0, 0, 0 };
    VROVector3f opacityStartMax = { 0, 0, 0 };
    std::shared_ptr<VROParticleModifier> opacity = std::make_shared<VROParticleModifier>(opacityStartMin,
                                                                                         opacityStartMax,
                                                                                         VROParticleModifier::VROModifierFactor::Time,
                                                                                         opacityIntervals);
    emitter->setAlphaModifier(opacity);
    
    // Velocity modifier
    std::vector<VROParticleModifier::VROModifierInterval> velocityIntervals;
    VROVector3f velocityStartMin = {  0.05, 0.15, 0.01 };
    VROVector3f velocityStartMax = { -0.05, 0.15, 0.01 };
    std::shared_ptr<VROParticleModifier> velocity = std::make_shared<VROParticleModifier>(velocityStartMin,
                                                                                          velocityStartMax,
                                                                                          VROParticleModifier::VROModifierFactor::Time,
                                                                                          velocityIntervals);
    emitter->setVelocityModifier(velocity);
    
    particleNode->setParticleEmitter(emitter);
    rootNode->addChildNode(particleNode);
    emitter->setRun(true);
}
