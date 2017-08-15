//
//  VROParticleEmitter.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROParticle.h"
#include "VROParticleModifier.h"
#include "VROParticleEmitter.h"
#include "VROSurface.h"
#include "VRONode.h"
#include "VROBillboardConstraint.h"
#include "VROParticleUBO.h"
#include "VROMaterial.h"

VROParticleEmitter::VROParticleEmitter(std::shared_ptr<VRODriver> driver,
                                       std::shared_ptr<VRONode> emitterNode,
                                       std::shared_ptr<VROTexture> texture,
                                        float width, float height) {

    // Create a particleUBO through which to batch particle information to the GPU.
    std::shared_ptr<VROParticleUBO> particleUBO = std::make_shared<VROParticleUBO>(driver);

    // Grab shader modifiers defined by the UBO for processing batched data.
    std::vector<std::shared_ptr<VROShaderModifier>> shaderModifiers = particleUBO->createInstanceShaderModifier();

    // Create a VROSurface, representing the particle geometry template for this emitter.
    std::shared_ptr<VROSurface> particleGeometry = VROSurface::createSurface(width, height, 0, 0, 1, 1);

    // Bind the Particle UBO to this geometry to be processed during instanced rendering.
    particleGeometry->setInstancedUBO(particleUBO);
    std::shared_ptr<VROMaterial> material = particleGeometry->getMaterials()[0];
    for (std::shared_ptr<VROShaderModifier> modifier : shaderModifiers) {
        material->addShaderModifier(modifier);
    }
    material->setWritesToDepthBuffer(false);
    material->setReadsFromDepthBuffer(false);

    // Apply the particle texture, if given.
    if (texture) {
        material->getDiffuse().setTexture(texture);
    }

    // Initialize the emitter with default values.
    initEmitter();

    // Finally, bind the particle geometry to the emitter node.
    emitterNode->setGeometry(particleGeometry);
    _particleEmitterNodeWeak = emitterNode;
}

VROParticleEmitter::~VROParticleEmitter() {}

void VROParticleEmitter::initEmitter() {
    // Default modifier behaviors
    _colorModifier = std::make_shared<VROParticleModifier>(VROVector3f(1,1,1));
    _scaleModifier = std::make_shared<VROParticleModifier>(VROVector3f(1,1,1));
    _rotationModifier = std::make_shared<VROParticleModifier>(VROVector3f(0,0,0));
    _alphaModifier = std::make_shared<VROParticleModifier>(VROVector3f(1,0,0));
    _velocityModifier = std::make_shared<VROParticleModifier>(VROVector3f(-2,6,0),
                                                              VROVector3f(2,6,0));
    _accelerationModifier = std::make_shared<VROParticleModifier>(VROVector3f(0,0,0),
                                                                  VROVector3f(0,0,0));

    // Default Emitter settings
    _duration = 2000;
    _maxParticles = 500;
    _particlesEmittedPerMeter = std::pair<int, int>(2, 2);
    _particlesEmittedPerSecond = std::pair<int, int>(10, 10);
    _particleLifeTime= std::pair<int, int>(2000,2000);
    _loop = true;
    _run = false;
    _requestRun = false;
}

void VROParticleEmitter::update(const VRORenderContext &context) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }

    double currentTime = VROTimeCurrentMillis();
    updateEmitter(currentTime, emitterNode);
    updateParticles(currentTime, context, emitterNode);
}

void VROParticleEmitter::updateEmitter(double currentTime, std::shared_ptr<VRONode> emitterNode) {
    if (_run != _requestRun) {
        _run = _requestRun;

        if (_run) {
            // If resuming the animation, recompute our startTime and startLocation.
            _emitterStartTimeMs = currentTime;
            _emitterStartLocation = emitterNode->getComputedTransform().extractTranslation();
        } else {
            // If pausing, preserve the amount of passed time and distance.
            _emitterPassedTimeSoFar = _emitterPassedTimeSoFar + (currentTime - _emitterStartTimeMs);

            VROVector3f currentLoc = emitterNode->getComputedTransform().extractTranslation();
            _emitterPassedDistanceSoFar = _emitterPassedDistanceSoFar
                                          + currentLoc.distance(_emitterStartLocation);
        }
    }

    // Process emitter states
    VROVector3f currentLoc = emitterNode->getComputedTransform().extractTranslation();
    _emitterTotalPassedTime = currentTime - _emitterStartTimeMs + _emitterPassedTimeSoFar;
    _emitterTotalPassedDistance = currentLoc.distance(_emitterStartLocation)
                                  + _emitterPassedDistanceSoFar;

    if (_emitterTotalPassedTime > _duration && _loop) {
        resetEmissionCycle(false, currentLoc);
    }
}

bool VROParticleEmitter::finishedEmissionCycle() {
    return _emitterTotalPassedTime > _duration && !_loop;
}

void VROParticleEmitter::resetEmissionCycle(bool resetParticles, VROVector3f currentLocation) {
    if (resetParticles) {
        _particles.clear();
        _zombieParticles.clear();
    }

    // Restart emitter times
    _emitterStartTimeMs = VROTimeCurrentMillis();
    _emitterTotalPassedTime = 0;
    _emitterPassedTimeSoFar = 0;

    // Restart location references for this emitter.
    _emitterTotalPassedDistance = 0;
    _emitterPassedDistanceSoFar = 0;
    _emitterStartLocation = currentLocation;

    // Restart this emitter's scheduled particle burst animation.
    std::vector<VROParticleBurst> copyBurst(_bursts);
    _scheduledBurst = copyBurst;

}

void VROParticleEmitter::updateParticles(double currentTime,
                                         const VRORenderContext &context,
                                         std::shared_ptr<VRONode> emitterNode) {
    updateParticlePhysics(currentTime);
    updateParticleAppearance(currentTime);
    updateParticlesToBeKilled(currentTime);
    if (!finishedEmissionCycle()) {
        updateParticleSpawn(currentTime, emitterNode->getComputedTransform().extractTranslation());
    }
    updateZombieParticles(currentTime);

    // Finally calculate and billboard the updated particle's world transform before rendering.
    VROMatrix4f emitterNodeTrans = emitterNode->getComputedTransform();
    std::shared_ptr<VROBillboardConstraint> constraint
            = std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All);

    for (int i = 0; i < _particles.size(); i ++) {
        if (_particles[i].fixedToEmitter) {
            // If the particle is fixedToEmitter, position the particle in referenceFactor to the
            // emitter's base transform.
            _particles[i].currentWorldTransform = emitterNodeTrans.multiply(_particles[i].currentLocalTransform);
        } else {
            // Else, position the particle in referenceFactor to where it had first been spawned.
            _particles[i].currentWorldTransform = _particles[i].spawnedWorldTransform.multiply(_particles[i].currentLocalTransform);
        }

        VROMatrix4f billboardRotation = constraint->getTransform(context, _particles[i].currentWorldTransform);
        VROVector3f computedPos = _particles[i].currentWorldTransform.extractTranslation();
        _particles[i].currentWorldTransform.translate(computedPos.scale(-1));
        _particles[i].currentWorldTransform = billboardRotation.multiply(_particles[i].currentWorldTransform);
        _particles[i].currentWorldTransform.translate(computedPos);
    }

    std::shared_ptr<VROInstancedUBO> instancedUBO = emitterNode->getGeometry()->getInstancedUBO();
    std::static_pointer_cast<VROParticleUBO>(instancedUBO)->update(_particles);
}

void VROParticleEmitter::updateParticlePhysics(double currentTime) {
    for (int i = 0; i < _particles.size(); i ++) {
        // Apply Physics modifiers
        _particles[i].timeSinceSpawnedInMs = currentTime - _particles[i].spawnTimeMs;
        VROVector3f velocity
                = _velocityModifier->applyModifier(_particles[i], _particles[i].initialVelocity);
        VROVector3f accel
                = _accelerationModifier->applyModifier(_particles[i],_particles[i].initialAccel);


        // Apply Velocity equation. NOTE: We are assuming a constant rate of acceleration and velocity.
        float tSec = _particles[i].timeSinceSpawnedInMs / 1000;
        VROVector3f displacement = velocity * tSec + accel * 0.5 * tSec * tSec;
        VROMatrix4f transform = _particles[i].spawnedLocalTransform;
        VROVector3f velocityFinal = velocity + accel * tSec;
        transform.translate(displacement);
        _particles[i].currentLocalTransform = transform;

        // Update distance, velocity and time deltas to be used for interpolating future modifiers.
        _particles[i].distanceTraveled = displacement.magnitude();
        _particles[i].velocity = velocityFinal.magnitude();
    }
}

void VROParticleEmitter::updateParticleAppearance(double currentTime) {
    for (int i = 0; i < _particles.size(); i ++) {
        if (_particles[i].isZombie) {
            continue;
        }
        VROVector3f alpha = _alphaModifier->applyModifier(_particles[i], _particles[i].initialAlpha);
        VROVector3f color = _colorModifier->applyModifier(_particles[i], _particles[i].initialColor);
        _particles[i].colorCurrent = VROVector4f(color.x, color.y, color.z, alpha.x);

        VROVector3f finalScale = _scaleModifier->applyModifier(_particles[i], _particles[i].initialScale);
        VROVector3f rotFinal = _rotationModifier->applyModifier(_particles[i], _particles[i].initialRotation);

        // Apply final transformation matrix and update particle.
        VROMatrix4f finalMatrix;
        finalMatrix.toIdentity();
        finalMatrix.translate(_particles[i].currentLocalTransform.extractTranslation());
        finalMatrix.scale(finalScale.x, finalScale.y, finalScale.z);
        finalMatrix.rotateX(rotFinal.x);
        finalMatrix.rotateY(rotFinal.y);
        finalMatrix.rotateZ(rotFinal.z);
        _particles[i].currentLocalTransform = finalMatrix;
    }
}

void VROParticleEmitter::updateParticleSpawn(double currentTime, VROVector3f currentPosition) {
    int totalParticles = 0;
    totalParticles = getSpawnParticlesPerSecond(currentTime);
    totalParticles += getSpawnParticlesPerMeter(currentPosition);
    totalParticles += getSpawnParticleBursts();

    // Determine if we've hit the max number of particles and return if so.
    int activeParticles = _particles.size() - _zombieParticles.size();
    if (totalParticles == 0 || activeParticles + totalParticles > _maxParticles) {
        return;
    }

    spawnParticle(totalParticles, currentTime);
}

int VROParticleEmitter::getSpawnParticlesPerSecond(double currentTime) {
    if (_particlesEmittedPerSecond.first == 0 && _particlesEmittedPerSecond.second == 0) {
        return 0;
    }

    int numOfParticlesToEmit = 0;
    double timeSinceLastEmit = currentTime - _intervalSpawnedLastEmitTime;
    if (_intervalSpawnedLastEmitTime != 0 && timeSinceLastEmit >= _intervalSpawnedEmissionRate) {
        // Calculate the number of particles to emit, and update lastEmitTime.
        double particles = floor(timeSinceLastEmit / _intervalSpawnedEmissionRate);
        _intervalSpawnedLastEmitTime = _intervalSpawnedLastEmitTime + (particles * _intervalSpawnedEmissionRate);
        numOfParticlesToEmit = particles;

        // Check if we passed  1000ms and if so remove additional particles that were
        // created in the overshot.
        if (_intervalSpawnedLastEmitTime - _intervalSpawnedInitTime > 1000) {
            double overShotTime = (_intervalSpawnedLastEmitTime - _intervalSpawnedInitTime) - 1000;
            int overShotParticles = overShotTime / _intervalSpawnedEmissionRate;
            numOfParticlesToEmit = numOfParticlesToEmit - overShotParticles;
        }
    }

    // Every second, recalculate the _intervalSpawnedEmissionRate (to accommodate a random rate).
    double timeSinceIntervalInit = currentTime - _intervalSpawnedInitTime;
    if (_intervalSpawnedInitTime == 0 || timeSinceIntervalInit > 1000) {
        double partsPerSec = ceil(random(_particlesEmittedPerSecond.first, _particlesEmittedPerSecond.second));
        _intervalSpawnedEmissionRate = 1000 / partsPerSec;
        _intervalSpawnedInitTime = currentTime;
        _intervalSpawnedLastEmitTime = currentTime;
        numOfParticlesToEmit = 0;
    }

    return numOfParticlesToEmit;
}

int VROParticleEmitter::getSpawnParticlesPerMeter(VROVector3f currentPos) {
    int numOfParticlesToSpawn = 0;
    double distanceTravelled = _distanceSpawnedLastEmitPosition.distance(currentPos);
    if (_distanceSpawnedEmissionRate != 0 && distanceTravelled > _distanceSpawnedEmissionRate) {
        numOfParticlesToSpawn = floor(distanceTravelled / _distanceSpawnedEmissionRate);
        _distanceSpawnedLastEmitPosition = currentPos;
    }

    float reRandomizationDistance = 1;
    if (_distanceSpawnedEmissionRate == 0
        || _distanceSpawnedInitPosition.distance(currentPos) > reRandomizationDistance) {
        double partsPerMeter = random(_particlesEmittedPerMeter.first, _particlesEmittedPerMeter.second);
        // Determine how much you have to travel per emitted particle.
        _distanceSpawnedEmissionRate = 1 / partsPerMeter;
        _distanceSpawnedLastEmitPosition = currentPos;
        _distanceSpawnedInitPosition = currentPos;
    }

    return numOfParticlesToSpawn;
}


int VROParticleEmitter::getSpawnParticleBursts(){
    if (_scheduledBurst.size() == 0){
        return 0;
    }

    int particlesToSpawn = 0;
    for (std::vector<VROParticleBurst>::iterator it=_scheduledBurst.begin(); it!=_scheduledBurst.end(); ) {
        VROParticleBurst &burst = *it;
        double currentCheckPoint;
        if (burst.referenceFactor == VROParticleModifier::VROModifierFactor::Time) {
            currentCheckPoint = _emitterTotalPassedTime;
        } else if (burst.referenceFactor == VROParticleModifier::VROModifierFactor::Distance) {
            currentCheckPoint = _emitterTotalPassedDistance;
        } else {
            pwarn("Error, attempted to process incorrectly configured burst!");
            continue;
        }

        if (currentCheckPoint > burst.referenceValueStart) {
            int max = burst.numberOfParticles.second;
            int min = burst.numberOfParticles.first;
            particlesToSpawn = particlesToSpawn + random(min, max);
            burst.referenceValueStart = burst.referenceValueStart + burst.referenceValueInterval;
            burst.cycles = burst.cycles -1;
            if (_bursts.size() == 0) {
            }
        }

        // Remove, if we have processed all its cycles.
        if (burst.cycles <= 0 ) {
            _scheduledBurst.erase(it);
        } else {
            ++it;
        }
    }

    return particlesToSpawn;
}

void VROParticleEmitter::spawnParticle(int numberOfParticles, double currentTime) {
    // Spawn by firstly re-cyling particles.
    int newParticles = numberOfParticles;
    for (std::vector<VROParticle>::iterator it=_zombieParticles.begin(); it!=_zombieParticles.end(); ) {
        VROParticle reInstatedParticle = *it;
        resetParticle(reInstatedParticle, currentTime);
        newParticles--;
        _particles.push_back(reInstatedParticle);
        _zombieParticles.erase(it);

        if (newParticles <=0) {
            return;
        }
    }

    // If there are not enough particles, create more, add it to the pool, else return.
    if (newParticles <= 0) {
        return;
    }

    for (int i = newParticles; i > 0; i --) {
        VROParticle particle;
        resetParticle(particle, currentTime);
        _particles.push_back(particle);
    }
}

void VROParticleEmitter::updateParticlesToBeKilled(double currentTime) {
    // _zombieParticles contains all particles that have died so that they can be reused.
    // Here, we add active particles that have died to this zombie list. Also note that particles
    // that become zombies will be de-allocated after a certain time.
    for (std::vector<VROParticle>::iterator it=_particles.begin(); it!=_particles.end(); ) {
        VROParticle particle = *it;
        bool hasDied = particle.spawnTimeMs + particle.lifePeriodMs < currentTime;
        if (hasDied) {
            particle.isZombie = true;
            particle.killedTimeMs = currentTime;
            _zombieParticles.push_back(particle);
            _particles.erase(it);
        } else {
            ++it;
        }
    }
}

void VROParticleEmitter::updateZombieParticles(double currentTime) {
    for (std::vector<VROParticle>::iterator it=_zombieParticles.begin(); it!=_zombieParticles.end(); ) {
        VROParticle zombieParticle = *it;
        if (zombieParticle.killedTimeMs + zombieParticle.zombiePeriodMs < currentTime) {
            _zombieParticles.erase(it);
        } else {
            ++it;
        }
    }
}

void VROParticleEmitter::resetParticle(VROParticle &particle, double currentTime) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }

    // Life cycle properties.
    particle.lifePeriodMs = random(_particleLifeTime.first, _particleLifeTime.second);
    particle.zombiePeriodMs = 500;
    particle.spawnTimeMs = currentTime;

    // Transformational properties.
    particle.spawnedWorldTransform = emitterNode->getComputedTransform();

    VROMatrix4f startTransform;
    startTransform.toIdentity();
    startTransform.scale(1, 1, 1);
    startTransform.translate({0, 0, 0});
    particle.spawnedLocalTransform = startTransform;
    particle.currentLocalTransform = startTransform;

    // Reset particle to initial defaults as defined by configured modifiers.
    particle.initialColor = _colorModifier->getInitialValue();
    particle.initialScale = _scaleModifier->getInitialValue();
    particle.initialRotation = _rotationModifier->getInitialValue();
    particle.initialVelocity = _velocityModifier->getInitialValue();
    particle.initialAccel = _accelerationModifier->getInitialValue();
    particle.initialAlpha = _alphaModifier->getInitialValue();

    // Set distance modifiers.
    particle.timeSinceSpawnedInMs = 0;
    particle.distanceTraveled = 0;
    particle.velocity = particle.initialVelocity.magnitude();

    particle.isZombie = false;
    particle.fixedToEmitter = _fixToEmitter;
}
