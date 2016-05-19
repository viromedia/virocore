//
//  VRODriverOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGL_h
#define VRODriverOpenGL_h

#include "VRODriver.h"
#include "VROGeometrySubstrateOpenGL.h"
#include "VROMaterialSubstrateOpenGL.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROVideoTextureCacheOpenGL.h"

class VRODriverOpenGL : public VRODriver {
    
public:
    
    VRODriverOpenGL(EAGLContext *eaglContext) :
        _eaglContext(eaglContext) {
        
    }
    
    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) const {
        return new VROGeometrySubstrateOpenGL(geometry, *this);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) const {
        return new VROMaterialSubstrateOpenGL(material, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<UIImage *> &images) const {
        return new VROTextureSubstrateOpenGL(type, images, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                             int width, int height) const {
        return new VROTextureSubstrateOpenGL(type, format, data, width, height, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(int width, int height, CGContextRef bitmapContext) const {
        return new VROTextureSubstrateOpenGL(width, height, bitmapContext, *this);
    }
    
    VROVideoTextureCache *newVideoTextureCache() const {
        return new VROVideoTextureCacheOpenGL(_eaglContext);
    }
    
private:
    
    EAGLContext *_eaglContext;
    
};

#endif /* VRODriverOpenGL_h */
