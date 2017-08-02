//
//  VROParticleEmitter.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROParticleEmitter.h"
#include "VROSurface.h"
#include "VRONode.h"
#include "VROBillboardConstraint.h"
#include "VROParticleUBO.h"

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

    // Finally, bind the particle geometry to the emitter node.
    emitterNode->setGeometry(particleGeometry);
    _particleEmitterNodeWeak = emitterNode;
}

VROParticleEmitter::~VROParticleEmitter() {}

void VROParticleEmitter::update(const VRORenderContext &context) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }

    double currentTime = VROTimeCurrentMillis();
    updateParticlePhysics(currentTime);
    updateParticleAppearance(currentTime);
    updateParticlesToBeKilled(currentTime);
    updateParticleSpawn(currentTime);
    updateZombieParticles(currentTime);

    // Finally calculate and billboard the updated particle's world transform before rendering.
    VROMatrix4f emitterNodeTrans = emitterNode->getComputedTransform();
    std::shared_ptr<VROBillboardConstraint> constraint
            = std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All);

    for (int i = 0; i < _particles.size(); i ++) {
        // If the particle is fixedToEmitter, position the particle in reference to the
        // emitter's base transform. Else, position the particle in reference to where it
        // had first been spawned.
        if (_particles[i].fixedToEmitter) {
            _particles[i].currentWorldTransform = emitterNodeTrans.multiply(_particles[i].currentLocalTransform);
        } else {
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
        double timeSinceSpawnedMs = currentTime - _particles[i].spawnTimeMs;
        float tSec = timeSinceSpawnedMs / 1000;

        VROVector3f velocity = _particles[i].velocity;
        VROVector3f accel = _particles[i].acceleration;
        VROVector3f displacement = velocity * tSec + accel * 0.5 * tSec * tSec;

        VROMatrix4f transform = _particles[i].startLocalTransform;
        transform.translate(displacement);
        _particles[i].currentLocalTransform = transform;
    }
}

void VROParticleEmitter::updateParticleAppearance(double currentTime) {
    for (int i = 0; i < _particles.size(); i ++) {
        if (_particles[i].isZombie) {
            continue;
        }

        // Grab the amount of passed time to interpolate the particle's properties with.
        double timeSinceSpawnedMs = currentTime - _particles[i].spawnTimeMs;
        double t = timeSinceSpawnedMs / _particles[i].lifePeriodMs;

        // Update color, scale and rotation
        _particles[i].colorCurrent = _particles[i].colorStart.interpolate(_particles[i].colorEnd, t);
        VROVector3f finalScale = _particles[i].startScale.interpolate(_particles[i].endScale, t);
        VROVector3f rotFinal =  _particles[i].startRotation.interpolate(_particles[i].endRotation, t);

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

void VROParticleEmitter::updateParticlesToBeKilled(double currentTime) {
    for (int i = 0; i < _particles.size(); i ++) {
        bool hasDied = _particles[i].spawnTimeMs + _particles[i].lifePeriodMs < currentTime;
        if (!_particles[i].isZombie && hasDied) {
            _particles[i].isZombie = true;
            _particles[i].killedTimeMs = currentTime;
            _zombieParticles.push_back(i);
        }
    }
}

void VROParticleEmitter::updateParticleSpawn(double currentTime) {
    // Spawn new particles if needed. First, determine if we've hit the spawn period
    bool shouldSpawn = true;
    double timeSinceLastSpawn = currentTime - _lastParticleSpawnTime;
    if (_lastParticleSpawnTime != 0 && timeSinceLastSpawn < _particlesSpawnIntervalMs) {
        shouldSpawn = false;
    }

    // Else, determine if we've hit the max number of particles.
    int activeParticles = _particles.size() - _zombieParticles.size();
    if (activeParticles + _particlesPerSpawn > _maxParticles) {
        shouldSpawn = false;
    }

    if (!shouldSpawn) {
        return;
    }

    _lastParticleSpawnTime = currentTime;

    // Spawn by firstly re-cyling particles.
    int newParticles = _particlesPerSpawn;
    for (std::vector<int>::iterator it=_zombieParticles.begin(); it!=_zombieParticles.end(); ) {
        resetParticle(_particles[*it], currentTime);
        newParticles--;
        _zombieParticles.erase(it);
    }

    // If there are not enough particles, create more, add it to the pool.
    if (newParticles <= 0) {
        return;
    }

    for (int i = newParticles; i > 0; i --) {
        VROParticle particle;
        resetParticle(particle, currentTime);
        _particles.push_back(particle);
    }
}

void VROParticleEmitter::updateZombieParticles(double currentTime) {
    for (std::vector<int>::iterator it=_zombieParticles.begin(); it!=_zombieParticles.end(); ) {
        int zombieIndex = *it;

        // If the particle has been in a zombie state for > expireGracePeriod, deallocate the particle.
        VROParticle zombieParticle = _particles[zombieIndex];
        if (zombieParticle.killedTimeMs + zombieParticle.zombiePeriodMs < currentTime) {
            _particles.erase(_particles.begin() + zombieIndex);
            _zombieParticles.erase(it);
        } else {
            ++it;
        }
    }
}

// TODO: VIRO-XXXX Remove hard coded constraints as we complete the Modifier Modules
void VROParticleEmitter::resetParticle(VROParticle &particle, double currentTime) {
    std::shared_ptr<VRONode> emitterNode = _particleEmitterNodeWeak.lock();
    if (emitterNode == nullptr) {
        return;
    }

    // Life cycle properties.
    particle.lifePeriodMs = 2000;
    particle.zombiePeriodMs = 100;
    particle.spawnTimeMs = currentTime;

    // Transformational properties.
    particle.spawnedWorldTransform = emitterNode->getComputedTransform();
    VROMatrix4f startTransform;
    startTransform.toIdentity();
    startTransform.scale(1, 1, 1);
    startTransform.translate({0, 0, 0});

    particle.startLocalTransform = startTransform;
    particle.currentLocalTransform = startTransform;

    float rotation = rand() % 12 - 6.28319;
    particle.startRotation = VROVector3f(0,0,0);
    particle.endRotation = VROVector3f(0,0,rotation);

    particle.startScale = VROVector3f(1,1,1);
    particle.endScale = VROVector3f(1.5,1.5,1.5);

    // Color properties
    particle.colorStart = VROVector4f(1,0,0,1);
    particle.colorEnd = VROVector4f(0,1,0,0);
    particle.colorCurrent = particle.colorStart.interpolate(particle.colorEnd, 0);

    // Physics properties
    float y = rand() % 8 + 6;
    float x = rand() % 5 - 2.5;
    float z = rand() % 5 - 2.5;

    VROVector3f randomVel = VROVector3f(x * 2, y * 2, z);
    randomVel.normalize();
    particle.velocity = randomVel;
    particle.acceleration = VROVector3f(0,-9.81, 0);
    particle.isZombie = false;
    particle.fixedToEmitter = true;
}
