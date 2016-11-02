//
//  VROTextureSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTextureSubstrateMetal_h
#define VROTextureSubstrateMetal_h

#include "VRODefines.h"
#if VRO_METAL

#include <UIKit/UIKit.h>
#include <Metal/Metal.h>
#include <vector>
#include "VROTextureSubstrate.h"
#include "VROAllocationTracker.h"

enum class VROTextureType;
enum class VROTextureFormat;
class VRODriver;
class VROData;

class VROTextureSubstrateMetal : public VROTextureSubstrate {
    
public:
    
    /*
     Create a new texture substrate with the given underlying MTLTexture.
     */
    VROTextureSubstrateMetal(id <MTLTexture> texture) :
        _texture(texture) {
    
        ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
    }
    
    /*
     Create a new Metal texture out of the contents of the current bitmap
     context.
     */
    VROTextureSubstrateMetal(int width, int height, CGContextRef bitmapContext,
                             VRODriver &driver);
    
    /*
     Create a new Metal texture of the given type from the given images.
     */
    VROTextureSubstrateMetal(VROTextureType type, std::vector<UIImage *> &images,
                             VRODriver &driver);
    
    /*
     Create a new Metal texture out of the given format, with the given width, and height.
     */
    VROTextureSubstrateMetal(VROTextureType type, VROTextureFormat format,
                             std::shared_ptr<VROData> data, int width, int height,
                             VRODriver &driver);
    virtual ~VROTextureSubstrateMetal();
    
    id <MTLTexture> getTexture() const {
        return _texture;
    }
    void setTexture(id <MTLTexture> texture) {
        _texture = texture;
    }
    
private:
  
    id <MTLTexture> _texture;
    
};

#endif
#endif /* VROTextureSubstrateMetal_h */
