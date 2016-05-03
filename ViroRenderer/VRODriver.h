//
//  VRODriver.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/21/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriver_h
#define VRODriver_h

#include <vector>
#import <UIKit/UIKit.h>

class VROGeometry;
class VROMaterial;
class VROGeometrySubstrate;
class VROMaterialSubstrate;
class VROTextureSubstrate;
class VROData;

enum class VROTextureType;
enum class VROTextureFormat;

/*
 The driver is used to interface with the rendering subsystem (OpenGL,
 Metal, etc.).
 */
class VRODriver {
    
public:
    
    virtual VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) const = 0;
    virtual VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<UIImage *> &images) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                                     int width, int height) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(int width, int height, CGContextRef bitmapContext) const = 0;
    
};

#endif /* VRODriver_hpp */
