//
//  VROARShadow.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/23/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARShadow.h"
#include "VROShaderModifier.h"
#include "VROMaterial.h"

static std::shared_ptr<VROShaderModifier> sShadowARSurfaceModifier;
static std::shared_ptr<VROShaderModifier> sShadowARLightingModifier;
static std::shared_ptr<VROShaderModifier> sShadowARFragmentModifier;

void VROARShadow::apply(std::shared_ptr<VROMaterial> material) {
    createSurfaceModifier();
    createLightingModifier();
    createFragmentModifier();

    material->setLightingModel(VROLightingModel::Lambert);
    if (!material->hasShaderModifier(sShadowARSurfaceModifier)) {
        material->addShaderModifier(sShadowARSurfaceModifier);
    }
    if (!material->hasShaderModifier(sShadowARLightingModifier)) {
        material->addShaderModifier(sShadowARLightingModifier);
    }
    if (!material->hasShaderModifier(sShadowARFragmentModifier)) {
        material->addShaderModifier(sShadowARFragmentModifier);
    }
}

void VROARShadow::remove(std::shared_ptr<VROMaterial> material) {
    if (sShadowARFragmentModifier != nullptr) {
        material->removeShaderModifier(sShadowARFragmentModifier);
    }
    if (sShadowARLightingModifier != nullptr) {
        material->removeShaderModifier(sShadowARLightingModifier);
    }
    if (sShadowARSurfaceModifier != nullptr) {
        material->removeShaderModifier(sShadowARSurfaceModifier);
    }
}

std::shared_ptr<VROShaderModifier> VROARShadow::createSurfaceModifier() {
    if (!sShadowARSurfaceModifier) {
        std::vector<std::string> modifierCode = {
            "highp float totalShadow = 0.0;",
        };
        sShadowARSurfaceModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                        modifierCode);
    }
    return sShadowARSurfaceModifier;
}

std::shared_ptr<VROShaderModifier> VROARShadow::createLightingModifier() {
    if (!sShadowARLightingModifier) {
        std::vector<std::string> modifierCode = {
            "totalShadow += (1.0 - _lightingContribution.visibility);",
        };
        sShadowARLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                        modifierCode);
        sShadowARLightingModifier->setName("arshadow");
    }
    return sShadowARLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROARShadow::createFragmentModifier() {
    if (!sShadowARFragmentModifier) {
        // Subtract ambient light from the shadow to simulate the softness that
        // comes from normal shadows when increasing ambient light. This does not
        // cover the softness that comes from other shadow-casting lights.
        std::vector<std::string> modifierCode = {
            "if (totalShadow != 0.0) {\n",
            "  _output_color = vec4(0, 0, 0, totalShadow - _ambient.r);\n",
            "} else {\n",
            "  _output_color = vec4(0, 0, 0, 0);\n",
            "}",
        };
        sShadowARFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                       modifierCode);
    }
    return sShadowARFragmentModifier;
}

