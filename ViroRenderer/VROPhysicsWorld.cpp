//
//  VROPhysicsWorld.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPhysicsWorld.h"
#include <btBulletDynamicsCommon.h>

static const float kPhysicsStepTime = 1 / 60.f;
static const float kPhysicsMaxSteps = 10;

VROPhysicsWorld::VROPhysicsWorld() {
    // Set to a default DbvtBroadphase (Dynamic AABB tree) as recommended by bullet.
    _broadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher with bullet defaults.
    _collisionConfiguration = new btDefaultCollisionConfiguration();
    _collisionDispatcher = new btCollisionDispatcher(_collisionConfiguration);
    _constraintSolver = new btSequentialImpulseConstraintSolver();

    // Finally, we create the physics world with the prepared configurations.
    _dynamicsWorld = new btDiscreteDynamicsWorld(_collisionDispatcher, _broadphase, _constraintSolver, _collisionConfiguration);

    // Default to Earth's gravity if none is set.
    _dynamicsWorld->setGravity({0,-9.81f,0});
}

VROPhysicsWorld::~VROPhysicsWorld() {
    delete _dynamicsWorld;
    delete _constraintSolver;
    delete _collisionDispatcher;
    delete _collisionConfiguration;
    delete _broadphase;
}

void VROPhysicsWorld::setGravity(VROVector3f gravity){
    _dynamicsWorld->setGravity({gravity.x, gravity.y, gravity.z});
}

void VROPhysicsWorld::addPhysicsBody(std::shared_ptr<VROPhysicsBody> body) {
    if (_activePhysicsBodies.find(body) != _activePhysicsBodies.end()){
        pwarn("Attempted to add the same physics body twice to the same physics world!");
        return;
    }

    _activePhysicsBodies.insert(body);
    btRigidBody* bulletBody = body->getBulletRigidBody();
    if (bulletBody){
        _dynamicsWorld->addRigidBody(bulletBody);
    } else {
        perror("Attempted to re-add a VROPhysics body with a mis-configured bulletBody!");
    }
}

void VROPhysicsWorld::removePhysicsBody(std::shared_ptr<VROPhysicsBody> body) {
    if (_activePhysicsBodies.find(body) == _activePhysicsBodies.end()){
        pwarn("Attempted to remove a VROPhysicsBody that does not exist in this physics world!");
        return;
    }

    _activePhysicsBodies.erase(body);
    btRigidBody* bulletBody = body->getBulletRigidBody();
    if (bulletBody && body->getIsSimulated()){
        _dynamicsWorld->removeRigidBody(bulletBody);
    } else {
        perror("Attempted to remove a VROPhysics body with a mis-configured bulletBody!");
    }
}

void VROPhysicsWorld::computePhysics() {
    std::set<std::shared_ptr<VROPhysicsBody>>::iterator it;
    for (it = _activePhysicsBodies.begin(); it != _activePhysicsBodies.end(); ++it) {
        std::shared_ptr<VROPhysicsBody> physicsBody = *it;

        // To update a bullet physics object, it must be removed and added back into the world.
        if (physicsBody->needsBulletUpdate()) {
            btRigidBody *oldBulletBody = physicsBody->getBulletRigidBody();
            _dynamicsWorld->removeRigidBody(oldBulletBody);

            physicsBody->updateBulletRigidBody();

            if (physicsBody->getIsSimulated()) {
                btRigidBody *newBulletBody = physicsBody->getBulletRigidBody();
                _dynamicsWorld->addRigidBody(newBulletBody);
            }
        }

        // Update gravity states for physics bodies
        if (!physicsBody->getUseGravity()){
            physicsBody->getBulletRigidBody()->setGravity({0,0,0});
        } else {
            physicsBody->getBulletRigidBody()->setGravity(_dynamicsWorld->getGravity());
        }

        physicsBody->updateBulletForces();
    }

    // Step through the physics simulation
    _dynamicsWorld->stepSimulation(kPhysicsStepTime, kPhysicsMaxSteps);
}
