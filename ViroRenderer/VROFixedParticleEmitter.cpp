//
//  VROFixedParticleEmitter.cpp
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROFixedParticleEmitter.h"
#include "VROImageUtil.h"
#include "VROSurface.h"
#include "VROMaterial.h"
#include "VROARSession.h"
#include "VROARFrame.h"
#include "VRONode.h"
#include "VROBillboardConstraint.h"
#include "VROParticleUBO.h"

VROFixedParticleEmitter::VROFixedParticleEmitter(){}
VROFixedParticleEmitter::~VROFixedParticleEmitter(){}
VROFixedParticleEmitter::VROFixedParticleEmitter(std::shared_ptr<VRODriver> driver) {
    initEmitter(driver, nullptr);
}

void VROFixedParticleEmitter::initEmitter(std::shared_ptr<VRODriver> driver,
                                          std::shared_ptr<VROSurface> particleGeometry) {
    // create default surfaces if needed.
    if (!particleGeometry) {
        particleGeometry = VROSurface::createSurface(.01, .01);
        particleGeometry->getMaterials()[0]->getDiffuse().setTexture(getPointCloudTexture());
        particleGeometry->getMaterials()[0]->setBloomThreshold(-1);
        particleGeometry->getMaterials()[0]->setBlendMode(VROBlendMode::Add);
    }

    // Create a particleUBO through which to batch particle information to the GPU.
    VROParticleEmitter::initParticleUBO(particleGeometry, driver);

    // Set FixedParticleEmitter defaults.
    _maxParticles = 2000;
    _particleScale = VROVector3f(1,1,1);
    _particleGeometry = particleGeometry;
}

void VROFixedParticleEmitter::forceClearParticles() {
    _particles.clear();
    _zombieParticles.clear();
    updateUBO(VROBoundingBox(0, 0, 0, 0, 0, 0));
}

void VROFixedParticleEmitter::setParticleTransforms(std::vector<VROVector4f> particleTransforms) {
    _particleComputedPositions = particleTransforms;
}

void VROFixedParticleEmitter::update(const VRORenderContext &context, const VROMatrix4f &computedTransform) {
    if (_particleComputedPositions.size() <= 0) {
        return;
    }

    VROBoundingBox boundingBox = updateParticles(_particleComputedPositions, computedTransform, context);
    updateUBO(boundingBox);
}

VROBoundingBox VROFixedParticleEmitter::updateParticles(std::vector<VROVector4f> particleTransforms,
                                                        const VROMatrix4f &baseTransform,
                                                        const VRORenderContext &context) {
    int pointCloudIndex = 0;
    int increment = 0;

    std::shared_ptr<VROBillboardConstraint> constraint = std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All);
    VROBoundingBox boundingBox = VROBoundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
    
    // repurpose existing particles in _particles array
    if (_particles.size() > 0) {
        increment = (int) std::min(particleTransforms.size(), _particles.size());

        for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
            computeParticleTransform(&_particles[i], particleTransforms[i], constraint, &boundingBox, baseTransform, context);
        }

        // if there were more _particles than point cloud points, zombify the rest and return
        if (_particles.size() >= particleTransforms.size()) {
            zombifyParticles(pointCloudIndex + increment);
            return boundingBox;
        }
    }

    // reuse zombie particles in _zombie particles
    if (_zombieParticles.size() > 0) {
        pointCloudIndex += increment;
        increment = (int) std::min(particleTransforms.size(), pointCloudIndex + _zombieParticles.size()) - pointCloudIndex;

        for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
            std::vector<VROParticle>::iterator it = _zombieParticles.end() - 1;
            VROParticle particle = *it;
            computeParticleTransform(&_particles[i], particleTransforms[i], constraint, &boundingBox, baseTransform, context);
            _particles.push_back(particle);
            _zombieParticles.erase(it);
        }

        // if each point cloud point has a corresponding particle, then return.
        if (particleTransforms.size() == _particles.size()) {
            return boundingBox;
        }
    }

    // at this point we've used up ALL zombies & particles, so we should create more particles!
    pointCloudIndex += increment;
    increment = std::min(_maxParticles, (int)particleTransforms.size()) - pointCloudIndex;

    for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
        VROParticle particle;
        computeParticleTransform(&particle, particleTransforms[i], constraint, &boundingBox, baseTransform, context);
        particle.colorCurrent = VROVector4f(1, 1, 1, 1);
        _particles.push_back(particle);
    }

    return boundingBox;
}

void VROFixedParticleEmitter::computeParticleTransform(VROParticle *particle,
                                                    VROVector4f position,
                                                    std::shared_ptr<VROBillboardConstraint> constraint,
                                                    VROBoundingBox *boundingBox,
                                                    const VROMatrix4f &baseTransform,
                                                    const VRORenderContext &context) {
    VROVector3f position3f = VROVector3f(position.x, position.y, position.z);

    particle->currentWorldTransform.toIdentity();
    particle->currentWorldTransform.scale(_particleScale.x, _particleScale.y, _particleScale.z);
    particle->currentWorldTransform.translate(position3f);

    VROMatrix4f worldTransform = baseTransform.multiply(particle->currentWorldTransform);
    particle->currentWorldTransform = worldTransform;

    VROMatrix4f billboardRotation = constraint->getTransform(context, particle->currentWorldTransform);
    VROVector3f computedPos = particle->currentWorldTransform.extractTranslation();
    particle->currentWorldTransform.translate(computedPos.scale(-1));
    particle->currentWorldTransform = billboardRotation.multiply(particle->currentWorldTransform);
    particle->currentWorldTransform.translate(computedPos);

    boundingBox->setMinX(std::min(boundingBox->getMinX(), computedPos.x));
    boundingBox->setMinY(std::min(boundingBox->getMinY(), computedPos.y));
    boundingBox->setMinZ(std::min(boundingBox->getMinZ(), computedPos.z));
    boundingBox->setMaxX(std::max(boundingBox->getMaxX(), computedPos.x));
    boundingBox->setMaxY(std::max(boundingBox->getMaxY(), computedPos.y));
    boundingBox->setMaxZ(std::max(boundingBox->getMaxZ(), computedPos.z));
}

void VROFixedParticleEmitter::updateUBO(VROBoundingBox boundingBox) {
    if (_particles.size() == 0) {
        boundingBox = VROBoundingBox(0,0,0,0,0,0);
    }
    std::shared_ptr<VROInstancedUBO> instancedUBO = _particleGeometry->getInstancedUBO();
    std::static_pointer_cast<VROParticleUBO>(instancedUBO)->update(_particles, boundingBox);
}

void VROFixedParticleEmitter::zombifyParticles(int startIndex) {
    std::vector<VROParticle>::iterator it = _particles.begin() + startIndex;
    while (it != _particles.end()) {
        VROParticle particle = *it;
        particle.isZombie = true;
        _zombieParticles.push_back(particle);
        _particles.erase(it);
    }
}
