//
//  VROARFrameARCore.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
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

#ifndef VROARFrameARCore_h
#define VROARFrameARCore_h

#include "VROCameraTexture.h"
#include "VROARFrame.h"
#include "VROViewport.h"
#include "ARCore_API.h"

class VROARSessionARCore;

class VROARFrameARCore : public VROARFrame {
public:
    
    VROARFrameARCore(arcore::Frame *frame, VROViewport viewport, std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARFrameARCore();
    
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    VROCameraOrientation getOrientation() const {
        return VROCameraOrientation::Portrait;
    }
    std::vector<std::shared_ptr<VROARHitTestResult>> hitTest(int x, int y, std::set<VROARHitTestResultType> types);
    std::vector<std::shared_ptr<VROARHitTestResult>> hitTestRay(VROVector3f *origin, VROVector3f *destination , std::set<VROARHitTestResultType> types);
    VROMatrix4f getViewportToCameraImageTransform() const;
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    VROVector3f getAmbientLightColor() const;

    bool hasDisplayGeometryChanged();
    void getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR);

    arcore::Frame *getFrameInternal() {
        return _frame;
    }

    std::shared_ptr<VROARPointCloud> getPointCloud();

private:

    arcore::Frame *_frame;
    std::weak_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROARCamera> _camera;
    VROViewport _viewport;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors; // Unused in ARCore
    std::shared_ptr<VROARPointCloud> _pointCloud;

};

#endif /* VROARFrameARCore_h */
