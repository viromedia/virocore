//
//  VRODriverOpenGLiOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLiOS_h
#define VRODriverOpenGLiOS_h

#include "VRODriverOpenGL.h"
#include "VROSoundEffectiOS.h"
#include "VROVideoTextureCacheOpenGL.h"

class VRODriverOpenGLiOS : public VRODriverOpenGL {
    
public:
    
    VRODriverOpenGLiOS(EAGLContext *eaglContext) :
        _eaglContext(eaglContext) {
        
    }
    
    virtual ~VRODriverOpenGLiOS() { }
    
    VROVideoTextureCache *newVideoTextureCache() {
        return new VROVideoTextureCacheOpenGL(_eaglContext);
    }
    
    std::shared_ptr<VROSoundEffect> newSoundEffect(std::string fileName) {
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName.c_str()]];
        std::string url = std::string([[fileURL description] UTF8String]);
        
        return std::make_shared<VROSoundEffectiOS>(url);
    }
    
private:
    
    EAGLContext *_eaglContext;
    
};

#endif /* VRODriverOpenGLiOS_h */
