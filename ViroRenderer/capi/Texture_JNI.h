//
//  Texture_JNI.h
//  ViroRenderer
//
//  Created by Andy Chu on 12/01/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef Texture_JNI_h
#define Texture_JNI_h

#include <memory>
#include "VROTexture.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Texture {

    VROTextureInternalFormat getFormat(VRO_ENV env, VRO_STRING jformat);
    VRO_OBJECT createJTexture(std::shared_ptr<VROTexture> texture);

}

#endif