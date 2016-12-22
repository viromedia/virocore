//
//  VROEventManager.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROEventManager_h
#define VROEventManager_h

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>
#include <set>
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VROEventDelegate.h"

/**
 * Responsible for mapping generalized input data from a controller, to a unified
 * set of VROEventDelegate.EventTypes. It then notifies corresponding VROEventDelegates
 * in the current scene about the event type that has been triggered.
 *
 * For example, the VROEventManager maps the onTouchPadClick form a Daydream Controller
 * to an onTap event type and notifies all VROEventDelegates about such an event.
 */
class VROEventManager{
public:
    VROEventManager(std::shared_ptr<VRORenderContext> context){
        _context = context;
        _gazedNode = nullptr;
        _scene = nullptr;
    }
    ~VROEventManager(){}

    /**
     * Generalized input data to be given by input controllers.
     */
    void onHeadGearTap();
    void onHeadGearGaze();

    /**
     * For notifying components outside the scene tree, we specifically register
     * them here to be tracked by the VROEventManager. Calling registerEventDelegate
     * twice with the same delegate will only have callbacks be triggered once.
     */
    void registerEventDelegate(std::shared_ptr<VROEventDelegate> delegate){
        _delegates.insert(delegate);
    }

    void removeEventDelegate(std::shared_ptr<VROEventDelegate> delegate){
        _delegates.erase(delegate);
    }

    /**
     * Sets the current scene for notifying events on nodes that have
     * both implemented and registered for an EventType.
     */
    void attachScene(std::shared_ptr<VROScene> scene){
        _scene = scene;
    }

    const float SCENE_BACKGROUND_DIST = 5.0f;

private:
    std::shared_ptr<VRONode> _gazedNode;
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VRORenderContext> _context;

    /**
     * Delegates registered within the manager to be notified of events
     * to an element that is outside the scene tree.
     */
    std::set<std::shared_ptr<VROEventDelegate>> _delegates;
    void processGazeEvent(std::shared_ptr<VRONode> node);

    VROHitTestResult hitTest(VROVector3f vector, VROVector3f hitFromPosition, bool boundsOnly);
    std::shared_ptr<VRONode> getNodeToHandleEvent(VROEventDelegate::EventType type,
                                                  std::shared_ptr<VRONode> startingNode);
};

#endif
