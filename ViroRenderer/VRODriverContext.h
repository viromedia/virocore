//
//  VRODriverContext.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/21/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverContext_h
#define VRODriverContext_h

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
 The driver context is used to share data between the driver and
 the rendering subsystem. This class contains driver-specific objects
 (such as Metal buffers and queues) for use by driver-specific substrates.
 */
class VRODriverContext {
    
public:
    
    virtual VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) const = 0;
    virtual VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<UIImage *> &images) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                                     int width, int height) const = 0;
    
};

#endif /* VRODriverContext_hpp */
