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
#include "VRODriverOpenGL.h"

VROParticleEmitter::VROParticleEmitter(std::shared_ptr<VRODriver> driver,
                                       std::shared_ptr<VRONode> emitterNode,
                                       std::shared_ptr<VROSurface> particleGeometry) {

    // Create a particleUBO through which to batch particle information to the GPU.
    std::shared_ptr<VROParticleUBO> particleUBO = std::make_shared<VROParticleUBO>(driver);

    // Grab shader modifiers defined by the UBO for processing batched data.
    std::vector<std::shared_ptr<VROShaderModifier>> shaderModifiers = particleUBO->createInstanceShaderModifier();

    // Bind the Particle UBO to this geometry to be processed during instanced rendering.
    particleGeometry->setInstancedUBO(particleUBO);
    std::shared_ptr<VROMaterial> material = particleGeometry->getMaterials()[0];
    for (std::shared_ptr<VROShaderModifier> modifier : shaderModifiers) {
        material->addShaderModifier(modifier);
    }
    material->setWritesToDepthBuffer(false);
    material->setReadsFromDepthBuffer(true);
    material->setBlendMode(VROBlendMode::Add);

    // Initialize the emitter with default values.
    initEmitter();

    // Finally, bind the particle geometry to the emitter node.
    emitterNode->setGeometry(particleGeometry);
    emitterNode->setIgnoreEventHandling(true);
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
    _accelerationModifier = std::make_shared<VROParticleModifier>(VROVector3f(0,0,0));

    // Default Emitter settings
    _emitterDelayDuration = -1;
    _emitterDelayStartTime = -1;
    _duration = 2000;
    _maxParticles = 500;
    _particlesEmittedPerMeter = std::pair<int, int>(0, 0);
    _particlesEmittedPerSecond = std::pair<int, int>(10, 10);
    _particleLifeTime= std::pair<int, int>(2000,2000);
    _loop = true;
    _run = false;
    _requestRun = false;
    _fixToEmitter = true;

    VROParticleSpawnVolume defaultVol;
    defaultVol.shape = VROParticleSpawnVolume::Shape::Point;
    _currentVolume = defaultVol;
}

bool VROParticleEmitter::processDelay(double currentTime) {
    // If no delay duration has been set, return immediately.
    if (_emitterDelayDuration == -1){
        return false;
    }

    // Set the delayStartTime for tracking delay status.
    if (_requestRun && _emitterDelayStartTime == -1) {
        _emitterDelayStartTime = currentTime;
        _emitterDelayTimePassedSoFar = _emitterDelayDuration;
    }
    // If pausing the emitter while still in the delay period, calculate and save the
    // processed delay time thus far, to be used for re-setting the delayStartTime when
    // resuming the emitter afterward.
    else if (!_requestRun && _emitterDelayStartTime != -1 &&
            _emitterDelayStartTime + _emitterDelayTimePassedSoFar > currentTime) {
        _emitterDelayStartTime = -1;
        _emitterDelayTimePassedSoFar = _emitterDelayStartTime - currentTime;
        return true;
    }

    // Return true if we are still in the delay period, false otherwise.
    if (_emitterDelayStartTime + _emitterDelayTimePassedSoFar > currentTime){
        return true;
    }

    return false;
}
void VROParticleEmitter::update(const VRORenderContext &context) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }
    double currentTime = VROTimeCurrentMillis();

    bool isCurrentlyDelayed = processDelay(currentTime);
    if (!isCurrentlyDelayed){
        updateEmitter(currentTime, emitterNode);
    }

    updateParticles(currentTime, context, emitterNode, isCurrentlyDelayed);
}

void VROParticleEmitter::updateEmitter(double currentTime, std::shared_ptr<VRONode> emitterNode) {
    if (_run != _requestRun || _emitterStartTimeMs == -1) {
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
        resetEmissionCycle(false);
    }
}

bool VROParticleEmitter::finishedEmissionCycle() {
    return _emitterTotalPassedTime > _duration && !_loop;
}

void VROParticleEmitter::resetEmissionCycle(bool resetParticles) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }

    if (resetParticles) {
        _particles.clear();
        _zombieParticles.clear();
    }

    // Restart delay times
    _emitterStartTimeMs = -1;
    _emitterDelayStartTime = -1;

    // Restart emitter times
    _emitterTotalPassedTime = 0;
    _emitterPassedTimeSoFar = 0;

    // Restart location references for this emitter.
    _emitterTotalPassedDistance = 0;
    _emitterPassedDistanceSoFar = 0;
    _emitterStartLocation = emitterNode->getComputedPosition();

    // Restart this emitter's scheduled particle burst animation.
    std::vector<VROParticleBurst> copyBurst(_bursts);
    _scheduledBurst = copyBurst;
}

void VROParticleEmitter::updateParticles(double currentTime,
                                         const VRORenderContext &context,
                                         std::shared_ptr<VRONode> emitterNode,
                                            bool isCurrentlyDelayed) {
    updateParticlePhysics(currentTime);
    updateParticleAppearance(currentTime);
    updateParticlesToBeKilled(currentTime);

    // Do not spawn if the emitter is in delay mode, or if !_run, or if we have finished
    // an emission cycle.
    if (!isCurrentlyDelayed && _run && !finishedEmissionCycle()) {
        updateParticleSpawn(currentTime, emitterNode->getComputedTransform().extractTranslation());
    }

    updateZombieParticles(currentTime);

    // Finally calculate and billboard the updated particle's world transform before rendering.
    VROMatrix4f emitterNodeTrans = emitterNode->getComputedTransform();
    std::shared_ptr<VROBillboardConstraint> constraint
            = std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All);

    float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX, minZ = FLT_MAX, maxZ = -FLT_MAX;
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

        minX = std::min(minX, computedPos.x);
        minY = std::min(minY, computedPos.y);
        minZ = std::min(minZ, computedPos.z);
        maxX = std::max(maxX, computedPos.x);
        maxY = std::max(maxY, computedPos.y);
        maxZ = std::max(maxZ, computedPos.z);
    }

    VROBoundingBox box =  VROBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
    if (_particles.size() == 0){
        box = VROBoundingBox(0,0,0,0,0,0);
    }
    std::shared_ptr<VROInstancedUBO> instancedUBO = emitterNode->getGeometry()->getInstancedUBO();
    std::static_pointer_cast<VROParticleUBO>(instancedUBO)->update(_particles, box);
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
        if (_impulseDeaccelerationExplosionPeriod != -1 && tSec > _impulseDeaccelerationExplosionPeriod){
            tSec = _impulseDeaccelerationExplosionPeriod;
        }

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

    VROVector3f initialScale = _scaleModifier->getInitialValue();
    VROMatrix4f startTransform;
    startTransform.toIdentity();
    startTransform.scale(initialScale.x, initialScale.y, initialScale.z);
    startTransform.translate(getPointInSpawnVolume());
    particle.spawnedLocalTransform = startTransform;
    particle.currentLocalTransform = startTransform;

    // Reset particle to initial defaults as defined by configured modifiers.
    particle.initialColor = _colorModifier->getInitialValue();
    particle.initialScale = initialScale;
    particle.initialRotation = _rotationModifier->getInitialValue();
    particle.initialAlpha = _alphaModifier->getInitialValue();

    if (_impulseExplosionMagnitude == -1){
        particle.initialVelocity = _velocityModifier->getInitialValue();
    } else {
        particle.initialVelocity = getExplosionInitialVel(startTransform.extractTranslation());
    }

    if (_impulseDeaccelerationExplosionPeriod == -1){
        particle.initialAccel = _accelerationModifier->getInitialValue();
    } else {
        _accelerationModifier
                = std::make_shared<VROParticleModifier>(getExplosionAccel(particle.initialVelocity));
        particle.initialAccel = _accelerationModifier->getInitialValue();
    }


    // Set distance modifiers.
    particle.timeSinceSpawnedInMs = 0;
    particle.distanceTraveled = 0;
    particle.velocity = particle.initialVelocity.magnitude();

    particle.isZombie = false;
    particle.fixedToEmitter = _fixToEmitter;
}

VROVector3f VROParticleEmitter::getPointInSpawnVolume() {
    if (_currentVolume.shape == VROParticleSpawnVolume::Shape::Box) {
        double width = 1 ,height = 1 ,length = 1;
        if (_currentVolume.shapeParams.size() != 3) {
            pwarn("Error: Provided incorrect parameters, defaulting to 1x1x1 Box.");
        } else {
            width = _currentVolume.shapeParams[0];
            height = _currentVolume.shapeParams[1];
            length = _currentVolume.shapeParams[2];
        }

        if (! _currentVolume.spawnOnSurface) {
            return VROVector3f(random(-width/2, width/2),
                               random(-height/2, height/2),
                               random(-length/2, length/2));
        } else {
            // Determine weighted distribution of each side
            double weightedSides[6];
            weightedSides[0] = width * height;
            weightedSides[1] = height * length;
            weightedSides[2] = length * width;
            weightedSides[3] = width * height;
            weightedSides[4] = height * length;
            weightedSides[5] = length * width;

            // Calculate total weight to randomize against to attain uniform distribution.
            double totalWeight = 0;
            for (double side : weightedSides) {
                totalWeight += side;
            }

            // Determine which side was picked
            double randomWeight = random(0,totalWeight);
            double weightSoFar = 0;
            int side = 0;
            for (side =0; side < 6; side ++) {
                weightSoFar += weightedSides[side];
                if (randomWeight <= weightSoFar) {
                    break;
                }
            }

            // Grab a point in the picked side and return that.
            int oneOfThree = side % 3;
            double x = 0, y = 0, z = 0;
            if (oneOfThree == 0) {
                x = random(-width/2, width/2);
                y = random(-height/2, height/2);
                z = side == 0 ? length/2 : -length/2;
            } else if (oneOfThree == 1) {
                x = side == 1 ? width/2 : -width/2;
                y = random(-height/2, height/2);
                z = random(-length/2, length/2);
            } else if (oneOfThree == 2) {
                x = random(-width/2, width/2);
                y = side == 2 ? height/2 : -height/2;
                z = random(-length/2, length/2);
            }
            return VROVector3f(x,y,z);
        }
    } else if (_currentVolume.shape == VROParticleSpawnVolume::Shape::Sphere) {
        double a = 1,b = 1,c = 1;
        if (_currentVolume.shapeParams.size() == 1) {
            // If ony defining radius, create a perfect sphere
            a = _currentVolume.shapeParams[0];
            b = _currentVolume.shapeParams[0];
            c = _currentVolume.shapeParams[0];
        } else if (_currentVolume.shapeParams.size() == 3) {
            // Else, set parameters for an ellipsoid
            a = _currentVolume.shapeParams[0];
            b = _currentVolume.shapeParams[1];
            c = _currentVolume.shapeParams[2];
        } else {
            pwarn("Unable to create sphere with incorrect params, defaulting to 1x1x1 sphere.");
        }

        // Grab a random x,y,z point from the ellipsoid, assuming a uniform distribution.
        float theta = 6.282 * random(0,1);
        float phi = acos (2 * random(0,1) - 1.0);
        float width = random(_currentVolume.spawnOnSurface? a : 0, a);
        float length = random(_currentVolume.spawnOnSurface? b : 0, b);
        float height = random(_currentVolume.spawnOnSurface? c : 0, c);

        double x = (width) * cos(theta) *sin(phi);
        double y = (length) * sin(theta)*sin(phi);
        double z = (height) * cos(phi);

        return VROVector3f(x,y,z);
    } else {
        // Default to a point.
        if (_currentVolume.shapeParams.size() == 3) {
            std::vector<double> p = _currentVolume.shapeParams;
            return VROVector3f(p[0], p[1], p[2]);
        }
    }
    return VROVector3f(0,0,0);
}

VROVector3f VROParticleEmitter::getExplosionInitialVel(VROVector3f particlePosition){
    if (_impulseExplosionMagnitude < 0){
        return VROVector3f(0,0,0);
    }

    VROVector3f blastDir = particlePosition - _explosionCenter;

    // Calculate applied impulse, that has an inverse square relationship to distance.
    float distance = particlePosition.distance(_explosionCenter);
    float invDistance = 1.0f / distance;
    float impulseMag = _impulseExplosionMagnitude * invDistance * invDistance;

    // Impulse equals to changes in momentum: Impulse = mass * DeltaVelocity
    VROVector3f impulse = blastDir * impulseMag;
    return impulse / kAssumedParticleMass;
}

VROVector3f VROParticleEmitter::getExplosionAccel(VROVector3f intialVelocity){
    // If we want an explosion with a de-accelerated force applied over time propotional
    // to where it had started from.
    return (intialVelocity / _impulseDeaccelerationExplosionPeriod) * -1.0;
}
