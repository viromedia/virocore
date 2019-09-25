//
//  VROSample.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include <VROARShadow.h>
#include "VROSample.h"
#include "VRODriverOpenGLAndroid.h"
#include "VRORendererTestHarness.h"
#include "VRORendererTest.h"
#include "VRONode.h"

VROSample::VROSample() {

}

VROSample::~VROSample() {

}

void VROSample::loadTestHarness(std::shared_ptr<VRORenderer> renderer,
                                std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                std::shared_ptr<VRODriver> driver) {
    _harness = std::make_shared<VRORendererTestHarness>(renderer, frameSynchronizer, driver);
    _harness->loadTest(VRORendererTestType::InverseKinematics);
}

std::shared_ptr<VROSceneController> VROSample::getSceneController() {
    return _harness->getCurrentTest()->getSceneController();
}

std::shared_ptr<VRONode> VROSample::getPointOfView() {
    return _harness->getCurrentTest()->getPointOfView();
}