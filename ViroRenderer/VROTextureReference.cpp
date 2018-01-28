//
//  VROTextureReference.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTextureReference.h"
#include "VROLog.h"
#include "VRORenderContext.h"

std::shared_ptr<VROTexture> VROTextureReference::getGlobalTexture(const VRORenderContext &context) const {
    switch (_globalType) {
        case VROGlobalTextureType::ShadowMap:
            return context.getShadowMap();
        case VROGlobalTextureType::IrradianceMap:
            return context.getIrradianceMap();
        case VROGlobalTextureType::PrefilteredMap:
            return context.getPreFilteredMap();
        case VROGlobalTextureType::BrdfMap:
            return context.getBRDFMap();
        default:
            pabort();
    }
}
