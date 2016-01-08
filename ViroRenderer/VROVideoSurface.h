//
//  VROVideoTexture.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROVideoTexture_h
#define VROVideoTexture_h

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CVMetalTextureCache.h>
#import "VROFrameListener.h"
#import "VROSurface.h"
#import <memory>

@class VROVideoCaptureDelegate;
class VRORenderContext;
class VROMaterial;
class VROSurface;

static const long kInFlightVideoTextures = 3;

class VROVideoSurface : public VROSurface, public VROFrameListener, public std::enable_shared_from_this<VROVideoSurface> {
    
public:
    
    static std::shared_ptr<VROVideoSurface> createVideoSurface(float width, float height);
    ~VROVideoSurface();
    
    /*
     Use this video surface to display the contents of the front-facing camera.
     */
    void captureFrontCamera(VRORenderContext &context);
    
    /*
     Use this video surface to display the contents of the given URL.
     */
    void displayVideo(NSURL *url, VRORenderContext &context);
    
    int getCurrentTextureIndex() const {
        return _currentTextureIndex;
    }
    CVMetalTextureCacheRef getVideoTextureCache() {
        return _videoTextureCache;
    }
    
    void onFrameWillRender() {}
    void onFrameDidRender() {
        _currentTextureIndex = (_currentTextureIndex + 1) % kInFlightVideoTextures;
    }
    
protected:
    
    VROVideoSurface(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                    std::vector<std::shared_ptr<VROGeometryElement>> &elements);
    
private:
    
    AVCaptureSession *_captureSession;
    CVMetalTextureCacheRef _videoTextureCache;
    
    VROVideoCaptureDelegate *_videoDelegate;
    int _currentTextureIndex;
    
};

@interface VROVideoCaptureDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithVROVideoSurface:(VROVideoSurface *)surface;

@end

#endif /* VROVideoTexture_h */
