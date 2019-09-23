//
//  VROPolylineTest.hpp
//  ViroKit
//
//  Created by Raj Advani on 10/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
