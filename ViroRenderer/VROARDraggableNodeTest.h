//
//  VROARDraggableNodeTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARDraggableNodeTest_h
#define VROARDraggableNodeTest_h

#include "VRORendererTest.h"

class VROARDraggableNodeEventDelegate : public VROEventDelegate {
public:
    VROARDraggableNodeEventDelegate() {};
    virtual ~VROARDraggableNodeEventDelegate() {};
    void onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition) {}
};

class VROARDraggableNodeTest : public VRORendererTest {
public:
    
    VROARDraggableNodeTest();
    virtual ~VROARDraggableNodeTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
private:

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARDraggableNodeEventDelegate> _eventDelegate;
    
};

#endif /* VROARDraggableNodeTest_h */
