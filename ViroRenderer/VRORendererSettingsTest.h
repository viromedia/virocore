//
//  VRORendererSettingsTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
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

#ifndef VRORendererSettingsTest_h
#define VRORendererSettingsTest_h

#include "VRORendererTest.h"

class VRORendererSettingsTest;

class VRORendererSettingsEventDelegate : public VROEventDelegate {
public:
    VRORendererSettingsEventDelegate(VRORendererSettingsTest *test) : _test(test) {};
    virtual ~VRORendererSettingsEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VRORendererSettingsTest *_test;
};

class VRORendererSettingsTest : public VRORendererTest {
public:
    
    VRORendererSettingsTest();
    virtual ~VRORendererSettingsTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void changeSettings();
    
private:

    int _index;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    
};

#endif /* VRORendererSettingsTest_h */
