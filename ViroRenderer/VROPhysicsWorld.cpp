//
//  VROPhysicsWorld.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPhysicsWorld.h"

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
        pwarn("Attempted to remove a VROPhsyicsBody that does not exist in this phsyics world!");
        return;
    }

    btRigidBody* bulletBody = body->getBulletRigidBody();
    if (bulletBody){
        _dynamicsWorld->removeRigidBody(bulletBody);
        _activePhysicsBodies.erase(body);
    } else {
        perror("Attempted to remove a VROPhysics body with a mis-configured bulletBody!");
    }
}

void VROPhysicsWorld::computePhysics() {
    std::set<std::shared_ptr<VROPhysicsBody>>::iterator it;
    for (it = _activePhysicsBodies.begin(); it != _activePhysicsBodies.end(); ++it) {
        std::shared_ptr<VROPhysicsBody> physicsBody = *it;

        // To update a bullet physics object, it must be removed and added back into the world.
        if (physicsBody->needsBulletUpdate()){
            btRigidBody *oldBulletBody = physicsBody->getBulletRigidBody();
            _dynamicsWorld->removeRigidBody(oldBulletBody);

            physicsBody->updateBulletRigidBody();
            btRigidBody *newBulletBody = physicsBody->getBulletRigidBody();
            _dynamicsWorld->addRigidBody(newBulletBody);
        }
    }

    // Step through the physics simulation
    _dynamicsWorld->stepSimulation(kPhysicsStepTime, kPhysicsMaxSteps);
}