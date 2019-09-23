//
//  VROInputControllerOVR.h
//  ViroRenderer
//
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

#ifndef VROInputControllerOVR_H
#define VROInputControllerOVR_H
#include <memory>
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROInputPresenterOVR.h"
#include <android/input.h>

class VROInputControllerOVR : public VROInputControllerBase {

public:
    VROInputControllerOVR(std::shared_ptr<VRODriver> driver) : VROInputControllerBase(driver) {}
    virtual ~VROInputControllerOVR(){}

    virtual VROVector3f getDragForwardOffset();
    void onProcess(const VROCamera &camera);
    void handleOVRKeyEvent(int keyCode, int action);
    void handleOVRTouchEvent(int touchAction, float posX, float posY);
    void updateSwipeGesture(VROVector3f start, VROVector3f end);

    virtual std::string getHeadset() {
        return "gearvr";
    }

    virtual std::string getController() {
        return "gearvr";
    }

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRODriver> driver) {
        return std::make_shared<VROInputPresenterOVR>();
    }

private:
    VROVector3f _touchDownLocationStart;
};
#endif
