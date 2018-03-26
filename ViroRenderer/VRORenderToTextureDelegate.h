//
//  VRORenderToTextureDelegate.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRO_RENDER_TO_TEXTURE_DELEGATE_H
#define VRO_RENDER_TO_TEXTURE_DELEGATE_H

#include <memory>

class VRORenderTarget;
class VRODriver;
class VROViewport;

/*
 The VRORenderToTextureDelegate is notified at the end of each frame with a VRORenderTarget
 that contains the fully rendered scene. This can be used to pass the rendered scene on to
 other processes.
 */
class VRORenderToTextureDelegate : public std::enable_shared_from_this<VRORenderToTextureDelegate> {
public:
    VRORenderToTextureDelegate() {};
    virtual ~VRORenderToTextureDelegate() {};

    /*
     Called to notify delegates with the final renderTarget containing the texture representing
     the output frame that is rendered on the screen.
     */
    virtual void renderedFrameTexture(std::shared_ptr<VRORenderTarget> renderedTarget,
                                      std::shared_ptr<VRODriver> driver) = 0;
};

#endif //VRO_RENDER_TO_TEXTURE_DELEGATE_H