//
//  VROSceneRendererGVR.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
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

#ifndef VRO_SCENE_RENDERER_GVR_H_  // NOLINT
#define VRO_SCENE_RENDERER_GVR_H_  // NOLINT

#include <UIKit/UIKit.h>
#include "VROOpenGL.h"
#include "VROMatrix4f.h"
#include <string>
#include <thread>
#include <vector>
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VRORenderer;
class VRODriver;
class VROViewport;
class VROFieldOfView;

class VROSceneRendererGVR : public std::enable_shared_from_this<VROSceneRendererGVR> {

public:

    VROSceneRendererGVR(int width, int height, UIInterfaceOrientation orientation,
                        float contentScaleFactor,
                        std::shared_ptr<VRORenderer> renderer,
                        std::shared_ptr<VRODriver> driver);
    virtual ~VROSceneRendererGVR();

    void initGL();
    void onDrawFrame();
    void onTouchEvent(int action, float x, float y);
    void recenterTracking();
    void refreshViewerProfile();
    
    void setVRModeEnabled(bool enabled);
    void setSurfaceSize(int width, int height, UIInterfaceOrientation orientation);
    void pause();
    void resume();

private:

    void createSwapchain();
    void renderStereo(VROMatrix4f &headView);
    void renderMono(VROMatrix4f &headView);

    int _frame;
    float _contentScaleFactor;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriver> _driver;
    
    std::unique_ptr<gvr::GvrApi> _gvr;
    std::unique_ptr<gvr::BufferViewportList> _viewportList;
    std::unique_ptr<gvr::SwapChain> _swapchain;

    // These are viewport *templates*. We use viewportList->SetBufferViewport to copy the
    // viewport into the appropriate position after configuring it each frame
    gvr::BufferViewport _hudViewport;
    gvr::BufferViewport _sceneViewport;

    gvr::Mat4f _headView;
    gvr::Sizei _surfaceSize;
    bool _sizeChanged;
    gvr::ViewerType _viewerType;

    bool _vrModeEnabled;
    double _suspendedNotificationTime;
    VROMatrix4f _orientationMatrix;

    /*
     Utility methods.
     */
    void setSurfaceSizeInternal(int width, int height, UIInterfaceOrientation orientation);
    void clearViewport(VROViewport viewport, bool transparent);
    gvr::Rectf modulateRect(const gvr::Rectf &rect, float width, float height);
    gvr::Recti calculatePixelSpaceRect(const gvr::Sizei &texture_size, const gvr::Rectf &texture_rect);
    void extractViewParameters(gvr::BufferViewport &viewport, gvr::Sizei renderSize,
                               VROViewport *outViewport,
                               VROFieldOfView *outFov);

};

#endif  // VRO_SCENE_RENDERER_GVR_H  // NOLINT
