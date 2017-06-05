//
//  VROPhysicsDebugDraw.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPhysicsDebugDraw_h
#define VROPhysicsDebugDraw_h

#include <btBulletDynamicsCommon.h>
#include "VROLog.h"
#include "VROMatrix4f.h"
#include "VRODriver.h"
#include "VRORenderParameters.h"
#include "VRONode.h"
#include "VROMaterial.h"
#include "VROGeometry.h"
#include "VROPolyline.h"

/*
 VROPhysicsDebugDraw is used by Bullet to render debug information of all VROPhysicsBodies
 within the VROPhysicsWorld, depending on the debugMode that is set.
 */
class VROPhysicsDebugDraw : public btIDebugDraw {
public:

    VROPhysicsDebugDraw(const std::shared_ptr<VROPencil> pencil) {
        _pencil = pencil;
    }
    virtual ~VROPhysicsDebugDraw(){}

    /*
     Called internally by Bullet to render lines when VROPhysicsWorld.debugDrawWorld() is called.
     */
    void drawLine(const btVector3& bulletFrom, const btVector3& bulletTo, const btVector3& bulletColor) {
        VROVector3f from = VROVector3f(bulletFrom.x(), bulletFrom.y(), bulletFrom.z());
        VROVector3f to = VROVector3f(bulletTo.x(), bulletTo.y(), bulletTo.z());
        VROVector4f color = VROVector4f({ bulletColor.getX(), bulletColor.getY(), bulletColor.getZ(), 1.0 });
        _pencil->draw(from, to);
    }

    /*
     Sets the mode that controls amount of debug information being displayed.
     See btIDebugDraw::DebugDrawMode for the different modes that can be set.
     */
    void setDebugMode(int debugMode) {
        _debugMode = debugMode;
    }

    /*
     Called internally by Bullet when determining the type of debug mode to render.
     */
    int getDebugMode() const {
        return _debugMode;
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
                          int lifeTime, const btVector3& color) {
        //No-op
    }

    void draw3dText(const btVector3& location,const char* textString) {
        //No-op
    }

    void reportErrorWarning(const char* warningString) {
        pwarn("VROPhysicsDebugDraw: %s", warningString);
    }

private:
    std::shared_ptr<VROPencil> _pencil;
    int _debugMode;
};
#endif
