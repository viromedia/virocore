//
//  VROPhysicsWorld.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <map>
#include "VROPhysicsWorld.h"
#include "VROPhysicsBodyDelegate.h"
#include <btBulletDynamicsCommon.h>
#include "VROPhysicsContactResultCallback.h"
#include "VROPhysicsDebugDraw.h"

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

    _debugDrawVisible = false;
    _debugDraw = nullptr;
}

VROPhysicsWorld::~VROPhysicsWorld() {
    _activePhysicsBodies.clear();
    delete _dynamicsWorld;
    delete _constraintSolver;
    delete _collisionDispatcher;
    delete _collisionConfiguration;
    delete _broadphase;

    if (_debugDraw != nullptr) {
        delete _debugDraw;
    }
}

void VROPhysicsWorld::setDebugDrawVisible(bool isVisible) {
    _debugDrawVisible = isVisible;
}

void VROPhysicsWorld::setGravity(VROVector3f gravity){
    _dynamicsWorld->setGravity({gravity.x, gravity.y, gravity.z});
}

void VROPhysicsWorld::addPhysicsBody(std::shared_ptr<VROPhysicsBody> body) {
    if (_activePhysicsBodies.find(body->getKey()) != _activePhysicsBodies.end()) {
        pwarn("Attempted to add the same physics body twice to the same physics world!");
        return;
    }

    _activePhysicsBodies[body->getKey()] = body;
    btRigidBody* bulletBody = body->getBulletRigidBody();
    if (bulletBody){
        _dynamicsWorld->addRigidBody(bulletBody);
    } else {
        perror("Attempted to re-add a VROPhysics body with a mis-configured bulletBody!");
    }
}

void VROPhysicsWorld::removePhysicsBody(std::shared_ptr<VROPhysicsBody> body) {
    if (_activePhysicsBodies.find(body->getKey()) == _activePhysicsBodies.end()) {
        pwarn("Attempted to remove a VROPhysicsBody that does not exist in this physics world!");
        return;
    }

    _activePhysicsBodies.erase(body->getKey());
    btRigidBody* bulletBody = body->getBulletRigidBody();
    if (bulletBody && body->getIsSimulated()){
        _dynamicsWorld->removeRigidBody(bulletBody);
    } else {
        perror("Attempted to remove a VROPhysics body with a mis-configured bulletBody!");
    }
}

void VROPhysicsWorld::computePhysics(const VRORenderContext &context) {
    // Update all VROPhysicsBodies as need be before the physics step.
    std::map<std::string, std::shared_ptr<VROPhysicsBody>>::iterator it;
    for (it = _activePhysicsBodies.begin(); it != _activePhysicsBodies.end(); ++it) {
        std::shared_ptr<VROPhysicsBody> physicsBody = it->second;

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
        if (!physicsBody->getUseGravity()) {
            physicsBody->getBulletRigidBody()->setGravity({0,0,0});
        } else {
            physicsBody->getBulletRigidBody()->setGravity(_dynamicsWorld->getGravity());
        }

        physicsBody->updateBulletForces();
        physicsBody->applyPresetVelocity();
    }

    // Step through the physics simulation
    _dynamicsWorld->stepSimulation(kPhysicsStepTime, kPhysicsMaxSteps);

    // Cycle through collisions that has resulted from the step.
    computeCollisions();

    // If debug draw is true, render a set of lines that represents the collision mesh of
    // every physics body contained within this world.
    if (_debugDrawVisible) {
        if (_debugDraw == nullptr) {
            _debugDraw = new VROPhysicsDebugDraw(context.getPencil());
            _debugDraw->setDebugMode(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);
            _dynamicsWorld->setDebugDrawer(_debugDraw);
        }
        _dynamicsWorld->debugDrawWorld();
    }
}

void VROPhysicsWorld::computeCollisions() {
    /*
     We generate collision pairs in the form of [bodyKeyA][bodyKeyB], where the array row
     [bodyKeyA] represents the body upon which to notify delegates about other bodies (array
     column [bodyKeyB]) that had collided against it. Thus we have a map representing these
     pairs of the form: map<physicsBodyKeyA, map<physicsBodyKeyB, VROCollision>>.
     */
    std::map<std::string, std::map<std::string, VROPhysicsBody::VROCollision>> collidedPairs;

    // Iterate through the dynamic world to generate collidedPairs from bullet's set of
    // collision pairs (manifolds) to be used for notifying our physics delegates.
    int numManifolds = _dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold =  _dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        int numContacts = contactManifold->getNumContacts();

        // Determine if any contact collidedPoints have been generated by the narrow phase
        // to ensure that a proper contact made collision has occurred.
        if (numContacts <=0) {
            continue;
        }

        // Sanity check ensuring our bullet / VROPhysics bodies are properly constructed
        const btCollisionObject *obA = contactManifold->getBody0();
        const btCollisionObject *obB = contactManifold->getBody1();
        if (obA->getUserPointer() == nullptr || obB->getUserPointer() == nullptr) {
            perror("Incorrectly constructed bullet rigid body for a VROPhysics body!");
            continue;
        }

        // Penetration depth given by bullet will be some negative number
        // if they are colliding. Here, we grab the point with shallowest penetration
        // depth to avoid providing geometry-penetrated points.
        float minPenetrationDistance = contactManifold->getContactPoint(0).getDistance();
        btManifoldPoint *bulletPoint = &contactManifold->getContactPoint(0);
        for (int j = 1; j < numContacts; j++) {

            btManifoldPoint *pt = &contactManifold->getContactPoint(j);
            float distance = pt->getDistance();
            if (distance < 0.1f && distance > minPenetrationDistance && pt->getLifeTime() > 10) {
                minPenetrationDistance = distance;
                bulletPoint = pt;
            }
        }

        // If there are no collided contact points for the given manifold, skip.
        if (minPenetrationDistance > 0.1f) {
            continue;
        }

        // Grab collision properties from bullet
        const btVector3& ptA = bulletPoint->getPositionWorldOnA();
        const btVector3& ptB = bulletPoint->getPositionWorldOnB();
        VROVector3f collisionOnBodyA = VROVector3f(ptA.x(), ptA.y(), ptA.z());
        VROVector3f collisionOnBodyB = VROVector3f(ptB.x(), ptB.y(), ptB.z());
        VROPhysicsBody *vroPhysicsBodyA = ((VROPhysicsBody *) obA->getUserPointer());
        VROPhysicsBody *vroPhysicsBodyB = ((VROPhysicsBody *) obB->getUserPointer());
        std::string bodyKeyA = vroPhysicsBodyA->getKey();
        std::string bodyKeyB = vroPhysicsBodyB->getKey();
        std::string bodyTagA = vroPhysicsBodyA->getTag();
        std::string bodyTagB = vroPhysicsBodyB->getTag();

        VROVector3f collidedNormal = VROVector3f(bulletPoint->m_normalWorldOnB.x(),
                                               bulletPoint->m_normalWorldOnB.y(),
                                               bulletPoint->m_normalWorldOnB.z());

        /*
         Update the properties of collision pairs in the form of [bodyKeyA][bodyKeyB], where
         the array row [bodyKeyA] represents the body upon which to notify delegates about
         other bodies (the array column [bodyKeyB]) that had collided against it (where
         it had collided, and it's corresponding normals). Thus, to account for notifying
         both collided bodies, we update both pairs as shown below. We also only save
         the shallowest penetration point for a given pair.
         */
        if (collidedPairs[bodyKeyA][bodyKeyB].penetrationDistance > minPenetrationDistance) {
            collidedPairs[bodyKeyA][bodyKeyB].collidedPoint = collisionOnBodyA;
            collidedPairs[bodyKeyA][bodyKeyB].penetrationDistance = minPenetrationDistance;
            collidedPairs[bodyKeyA][bodyKeyB].collidedBodyTag = bodyTagB;
            collidedPairs[bodyKeyA][bodyKeyB].collidedNormal = collidedNormal;
        }

        if (collidedPairs[bodyKeyB][bodyKeyA].penetrationDistance > minPenetrationDistance) {
            collidedPairs[bodyKeyB][bodyKeyA].collidedPoint = collisionOnBodyB;
            collidedPairs[bodyKeyB][bodyKeyA].penetrationDistance = minPenetrationDistance;
            collidedPairs[bodyKeyB][bodyKeyA].collidedBodyTag = bodyTagA;
            collidedPairs[bodyKeyB][bodyKeyA].collidedNormal = collidedNormal;
        }
    }

    /*
     Iterate through active vroPhysicsBodies and notify those with attached physics delegates about
     the latest map of collided bodies and it's corresponding VROCollision.
     */
    std::map<std::string, std::shared_ptr<VROPhysicsBody>>::iterator it_body;
    for (it_body = _activePhysicsBodies.begin(); it_body != _activePhysicsBodies.end(); ++it_body) {
        std::shared_ptr<VROPhysicsBody> physicsBodyA = it_body->second;
        std::shared_ptr<VROPhysicsBodyDelegate> delegateA = physicsBodyA->getPhysicsDelegate();
        if (!delegateA){
            continue;
        }

        std::string key = physicsBodyA->getKey();
        delegateA->onEngineCollisionUpdate(key, collidedPairs[key]);
        collidedPairs[key].clear();
    }

    collidedPairs.clear();
}

bool VROPhysicsWorld::findCollisionsWithRay(VROVector3f fromPos, VROVector3f toPos,
                                            bool returnClosest,
                                            std::string rayTag){
    btVector3 from = {fromPos.x, fromPos.y, fromPos.z};
    btVector3 to = {toPos.x, toPos.y, toPos.z};

    // Use Bullet's ClosestRayResultCallback if returnClosest is true
    if (returnClosest) {
        btCollisionWorld::ClosestRayResultCallback result(from, to);
        _dynamicsWorld->rayTest(from, to, result);

        // Return immediately if nothing has hit.
        if (!result.hasHit()) {
            return false;
        }

        // Sanity check ensuring our Bullet / VROPhysics bodies are properly constructed
        if (result.m_collisionObject->getUserPointer() == nullptr) {
            perror("Incorrectly constructed bullet rigid body for a VROPhysics body!");
            return false;
        }

        // If available, notify physics delegates about collisions.
        VROPhysicsBody *physicsObj = ((VROPhysicsBody *) result.m_collisionObject->getUserPointer());
        std::shared_ptr<VROPhysicsBodyDelegate> delgate = physicsObj->getPhysicsDelegate();
        if (delgate != nullptr) {
            VROPhysicsBody::VROCollision obj;
            obj.collidedPoint = VROVector3f(result.m_hitPointWorld.x(),
                                            result.m_hitPointWorld.y(),
                                            result.m_hitPointWorld.z());

            obj.collidedNormal = VROVector3f(result.m_hitNormalWorld.x(),
                                             result.m_hitNormalWorld.y(),
                                             result.m_hitNormalWorld.z());
            obj.collidedBodyTag = rayTag;
            delgate->onCollided(physicsObj->getKey(), obj);
        }
    }
    // Else, the user wants all the hit results; use Bullet's AllHitsRayResultCallback
    else {
        btCollisionWorld::AllHitsRayResultCallback results(from, to);
        _dynamicsWorld->rayTest(from, to, results);

        // Return immediately if nothing has hit.
        if (!results.hasHit()) {
            return false;
        }

        // Sanity check to ensure the resulting collision vectors match up (they should).
        passert(results.m_hitPointWorld.size() == results.m_hitNormalWorld.size());
        passert(results.m_collisionObjects.size() == results.m_hitPointWorld.size());

        // Iterate through each collided object and notify delegates.
        for (int i=0;i<results.m_collisionObjects.size();i++) {

            // Sanity check ensuring our Bullet / VROPhysics bodies are properly constructed
            if (results.m_collisionObjects[i]->getUserPointer() == nullptr) {
                perror("Incorrectly constructed bullet rigid body for a VROPhysics body!");
                return false;
            }

            VROPhysicsBody *physicsObj = ((VROPhysicsBody *) results.m_collisionObjects[i]->getUserPointer());
            std::shared_ptr<VROPhysicsBodyDelegate> delgate = physicsObj->getPhysicsDelegate();
            if (delgate != nullptr) {
                VROPhysicsBody::VROCollision obj;
                obj.collidedPoint = VROVector3f(results.m_hitPointWorld[i].x(),
                                                results.m_hitPointWorld[i].y(),
                                                results.m_hitPointWorld[i].z());

                obj.collidedNormal = VROVector3f(results.m_hitNormalWorld[i].x(),
                                                 results.m_hitNormalWorld[i].y(),
                                                 results.m_hitNormalWorld[i].z());
                obj.collidedBodyTag = rayTag;
                delgate->onCollided(physicsObj->getKey(), obj);
            }
        }
    }
    return true;
}

/*
 Performs a collision test by projecting a collision shape into the scene. If checking along a path,
 only the first collided object is notified. Else, all collided objects intersecting the shape
 are notified.
 */
bool VROPhysicsWorld::findCollisionsWithShape(VROVector3f fromPos, VROVector3f toPos,
                                              std::shared_ptr<VROPhysicsShape> shape,
                                              std::string rayTag) {
    if (fromPos == toPos) {
        // We are performing a shapeCollision test at a point
        return collisionTestAtPoint(fromPos, shape, rayTag);
    } else {
        // We are performing a shapeCollision test with start and end points.
        return collisionTestAlongPath(fromPos, toPos, shape, rayTag);
    }
}

bool VROPhysicsWorld::collisionTestAtPoint(VROVector3f pos,
                                           std::shared_ptr<VROPhysicsShape> shape,
                                           std::string rayTag) {
    btVector3 position = {pos.x, pos.y, pos.z};
    btCollisionShape *bulletShape = shape->getBulletShape();
    if (!bulletShape) {
        perr("Attempted to perform a raycast with an invalid VROPhysicsShape!");
        return false;
    }

    // Create a temporary collision object with the provided shape, and add it to the dynamic
    // world to perform the collision test with.
    btCollisionObject* col = new btCollisionObject();
    col->setCollisionShape(bulletShape);
    col->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
    col->setWorldTransform(btTransform(btQuaternion::getIdentity(), position));
    _dynamicsWorld->addCollisionObject(col);

    // Perform the collision contact test
    VROPhysicsContactResultCallback results;
    _dynamicsWorld->contactTest(col, results);

    // Notify the corresponding delegates of collided objects.
    for (VROPhysicsBody::VROCollision collision : results._collisions) {
        if (_activePhysicsBodies.count(collision.collidedBodyTag) == 0){
            continue;
        }
        
        std::shared_ptr<VROPhysicsBody> body = _activePhysicsBodies[collision.collidedBodyTag];
        std::shared_ptr<VROPhysicsBodyDelegate> delegate = body->getPhysicsDelegate();
        if (delegate == nullptr) {
            continue;
        }
        
        collision.collidedBodyTag = rayTag;
        delegate->onCollided(body->getKey(), collision);
    }

    // After the collision tests are done, remove our temporary collision object.
    _dynamicsWorld->removeCollisionObject(col);
    delete(col);

    return results._collisions.size() > 0;
}

bool VROPhysicsWorld::collisionTestAlongPath(VROVector3f fromPos, VROVector3f toPos,
                                             std::shared_ptr<VROPhysicsShape> shape,
                                             std::string rayTag) {
    btVector3 from = {fromPos.x, fromPos.y, fromPos.z};
    btVector3 to = {toPos.x, toPos.y, toPos.z};
    btCollisionShape *bulletShape = shape->getBulletShape();
    if (!bulletShape) {
        perr("Attempted to perform a raycast with an invalid VROPhysicsShape!");
        return false;
    }
    
    // Construct the start and end transform that represents the collision path.
    btTransform fromTrans = btTransform(btQuaternion::getIdentity(), btVector3(fromPos.x, fromPos.y, fromPos.z));
    btTransform toTrans = btTransform(btQuaternion::getIdentity(), btVector3(toPos.x, toPos.y, toPos.z));

    // Perform the ray cast with the convex shape.
    btConvexShape *convexShape = (btConvexShape *)bulletShape;
    btCollisionWorld::ClosestConvexResultCallback result(from, to);
    _dynamicsWorld->convexSweepTest(convexShape, fromTrans, toTrans, result);

    // Return immediately if nothing has hit.
    if (!result.hasHit()) {
        return false;
    }

    // Sanity check ensuring our Bullet / VROPhysics bodies are properly constructed
    if (result.m_hitCollisionObject->getUserPointer() == nullptr) {
        perror("Incorrectly constructed bullet rigid body for a VROPhysics body!");
        return false;
    }

    // Notify physics delegates about raycast collision results if available.
    VROPhysicsBody *physicsObj = ((VROPhysicsBody *) result.m_hitCollisionObject->getUserPointer());
    std::shared_ptr<VROPhysicsBodyDelegate> delgate = physicsObj->getPhysicsDelegate();
    if (delgate != nullptr) {
        VROPhysicsBody::VROCollision obj;
        obj.collidedPoint = VROVector3f(result.m_hitPointWorld.x(),
                                        result.m_hitPointWorld.y(),
                                        result.m_hitPointWorld.z());
            
        obj.collidedNormal = VROVector3f(result.m_hitNormalWorld.x(),
                                         result.m_hitNormalWorld.y(),
                                         result.m_hitNormalWorld.z());
        obj.collidedBodyTag = rayTag;
        delgate->onCollided(physicsObj->getKey(), obj);
    }
    return true;
}
