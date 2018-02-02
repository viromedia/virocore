//
//  VROPolylineTest.hpp
//  ViroKit
//
//  Created by Raj Advani on 10/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPolylineTest_hpp
#define VROPolylineTest_hpp

#include "VRORendererTest.h"

class VROPolylineEventDelegate : public VROEventDelegate {
public:
    VROPolylineEventDelegate(std::shared_ptr<VROPolyline> polyline) : _polyline(polyline) {};
    virtual ~VROPolylineEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    void onMove(int source, std::shared_ptr<VRONode> node, VROVector3f rotation, VROVector3f position, VROVector3f forwardVec);
    void onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition);
    
private:
    std::weak_ptr<VROPolyline> _polyline;
};

class VROPolylineTest : public VRORendererTest {
public:
    
    VROPolylineTest();
    virtual ~VROPolylineTest();
    
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
    
    std::shared_ptr<VROPolyline> _polyline;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    
};

#endif /* VROPolylineTest_hpp */
