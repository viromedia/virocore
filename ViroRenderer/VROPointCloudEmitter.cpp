//
//  VROPointCloudEmitter.cpp
//  ViroKit
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPointCloudEmitter.h"
#include "VROImageUtil.h"
#include "VROSurface.h"
#include "VROMaterial.h"
#include "VROARSession.h"
#include "VROARFrame.h"
#include "VRONode.h"
#include "VROBillboardConstraint.h"
#include "VROParticleUBO.h"

VROPointCloudEmitter::VROPointCloudEmitter(std::shared_ptr<VRODriver> driver,
                                           std::shared_ptr<VROARSession> session) :
    _particleScale(VROVector3f(.01, .01, .01)) {
    initPointCloudTexture();

    _arSession = session;

    // create surfaces w/ size 1,1 and use _particleScale to downsize (if required).
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1, 1);
    surface->getMaterials()[0]->getDiffuse().setTexture(getPointCloudTexture());
    surface->getMaterials()[0]->setBloomThreshold(-1);
    surface->getMaterials()[0]->setBlendMode(VROBlendMode::Add);

    VROParticleEmitter::initEmitter(driver, surface);
}

void VROPointCloudEmitter::clearParticles() {
    _particles.clear();
    _zombieParticles.clear();
    updateUBO(VROBoundingBox(0, 0, 0, 0, 0, 0));
}

void VROPointCloudEmitter::resetParticleSurface() {
    std::shared_ptr<VROSurface> newSurface = VROSurface::createSurface(1, 1);
    newSurface->getMaterials()[0]->getDiffuse().setTexture(getPointCloudTexture());
    newSurface->getMaterials()[0]->setBloomThreshold(-1);
    newSurface->getMaterials()[0]->setBlendMode(VROBlendMode::Add);
    VROParticleEmitter::setParticleSurface(newSurface);
}

void VROPointCloudEmitter::setParticleSurface(std::shared_ptr<VROSurface> particleSurface) {
    VROParticleEmitter::setParticleSurface(particleSurface);
}

void VROPointCloudEmitter::update(const VRORenderContext &context, const VROMatrix4f &computedTransform) {
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (!arSession) {
        return;
    }
    std::unique_ptr<VROARFrame> &frame = arSession->getLastFrame();
    if (!frame) {
        return;
    }
    
    std::vector<VROVector4f> pointCloudPoints = frame->getPointCloud()->getPoints();
    VROBoundingBox boundingBox = updateParticles(pointCloudPoints, context);
    updateUBO(boundingBox);
}

VROBoundingBox VROPointCloudEmitter::updateParticles(std::vector<VROVector4f> pointCloudPoints,
                                                     const VRORenderContext &context) {
    int pointCloudIndex = 0;
    int increment = 0;
    
    std::shared_ptr<VROBillboardConstraint> constraint = std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All);
    VROBoundingBox boundingBox = VROBoundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
    
    // repurpose existing particles in _particles array
    if (_particles.size() > 0) {
        increment = (int) std::min(pointCloudPoints.size(), _particles.size());

        for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
            computeParticleTransform(&_particles[i], pointCloudPoints[i], constraint, &boundingBox, context);
        }

        // if there were more _particles than point cloud points, zombify the rest and return
        if (_particles.size() >= pointCloudPoints.size()) {
            zombifyParticles(pointCloudIndex + increment);
            return boundingBox;
        }
    }

    // reuse zombie particles in _zombie particles
    if (_zombieParticles.size() > 0) {
        pointCloudIndex += increment;
        increment = (int) std::min(pointCloudPoints.size(), pointCloudIndex + _zombieParticles.size()) - pointCloudIndex;

        for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
            std::vector<VROParticle>::iterator it = _zombieParticles.end() - 1;
            VROParticle particle = *it;
            computeParticleTransform(&_particles[i], pointCloudPoints[i], constraint, &boundingBox, context);
            _particles.push_back(particle);
            _zombieParticles.erase(it);
        }

        // if each point cloud point has a corresponding particle, then return.
        if (pointCloudPoints.size() == _particles.size()) {
            return boundingBox;
        }
    }

    // at this point we've used up ALL zombies & particles, so we should create more particles!
    pointCloudIndex += increment;
    increment = std::min(_maxParticles, (int)pointCloudPoints.size()) - pointCloudIndex;

    for (int i = pointCloudIndex; i < pointCloudIndex + increment; i++) {
        VROParticle particle;
        computeParticleTransform(&particle, pointCloudPoints[i], constraint, &boundingBox, context);
        particle.colorCurrent = VROVector4f(1, 1, 1, 1);
        _particles.push_back(particle);
    }

    return boundingBox;
}

void VROPointCloudEmitter::computeParticleTransform(VROParticle *particle,
                                                    VROVector4f position,
                                                    std::shared_ptr<VROBillboardConstraint> constraint,
                                                    VROBoundingBox *boundingBox,
                                                    const VRORenderContext &context) {
    VROVector3f position3f = VROVector3f(position.x, position.y, position.z);

    particle->currentWorldTransform.toIdentity();
    particle->currentWorldTransform.scale(_particleScale.x, _particleScale.y, _particleScale.z);
    particle->currentWorldTransform.translate(position3f);

    VROMatrix4f billboardRotation = constraint->getTransform(context, particle->currentWorldTransform);
    particle->currentWorldTransform.translate(position3f.scale(-1));
    particle->currentWorldTransform = billboardRotation.multiply(particle->currentWorldTransform);
    particle->currentWorldTransform.translate(position3f);

    boundingBox->setMinX(std::min(boundingBox->getMinX(), position3f.x));
    boundingBox->setMinY(std::min(boundingBox->getMinY(), position3f.y));
    boundingBox->setMinZ(std::min(boundingBox->getMinZ(), position3f.z));
    boundingBox->setMaxX(std::max(boundingBox->getMaxX(), position3f.x));
    boundingBox->setMaxY(std::max(boundingBox->getMaxY(), position3f.y));
    boundingBox->setMaxZ(std::max(boundingBox->getMaxZ(), position3f.z));
}

void VROPointCloudEmitter::updateUBO(VROBoundingBox boundingBox) {
    if (_particles.size() == 0){
        boundingBox = VROBoundingBox(0,0,0,0,0,0);
    }
    std::shared_ptr<VROInstancedUBO> instancedUBO = _particleGeometry->getInstancedUBO();
    std::static_pointer_cast<VROParticleUBO>(instancedUBO)->update(_particles, boundingBox);
}

void VROPointCloudEmitter::zombifyParticles(int startIndex) {
    std::vector<VROParticle>::iterator it = _particles.begin() + startIndex;
    while (it != _particles.end()) {
        VROParticle particle = *it;
        particle.isZombie = true;
        _zombieParticles.push_back(particle);
        _particles.erase(it);
    }
}
