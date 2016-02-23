//
//  VROVideoTexture.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROVideoSurface_h
#define VROVideoSurface_h

#import "VROSurface.h"
#import <memory>

class VRORenderContext;
class VROMaterial;
class VROSurface;
class VROVideoTexture;

class VROVideoSurface : public VROSurface {
    
public:
    
    static std::shared_ptr<VROVideoSurface> createVideoSurface(float width, float height,
                                                               NSURL *url, VRORenderContext &context);
    
    void pause();
    void play();
    bool isPaused();
    
    ~VROVideoSurface();
    
private:
    
    VROVideoSurface(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                    std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                    std::shared_ptr<VROVideoTexture> texture);
    
    std::shared_ptr<VROVideoTexture> _texture;
    
};

#endif /* VROVideoSurface_h */
