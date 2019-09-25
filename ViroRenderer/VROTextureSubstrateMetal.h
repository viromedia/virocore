//
//  VROTextureSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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
class VROImage;

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
     Create a new Metal texture of the given type from the given images.
     */
    VROTextureSubstrateMetal(VROTextureType type, std::vector<std::shared_ptr<VROImage>> &images,
                             std::shared_ptr<VRODriver> &driver);
    
    /*
     Create a new Metal texture out of the given format, with the given width, and height.
     */
    VROTextureSubstrateMetal(VROTextureType type, VROTextureFormat format,
                             std::shared_ptr<VROData> data, int width, int height,
                             std::shared_ptr<VRODriver> &driver);
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
