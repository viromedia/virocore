//
//  VROARShadow.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/23/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "VROARShadow.h"
#include "VROShaderModifier.h"
#include "VROMaterial.h"

static thread_local std::shared_ptr<VROShaderModifier> sShadowARSurfaceModifier;
static thread_local std::shared_ptr<VROShaderModifier> sShadowARLightingModifier;
static thread_local std::shared_ptr<VROShaderModifier> sShadowARFragmentModifier;

void VROARShadow::apply(std::shared_ptr<VROMaterial> material) {
    createSurfaceModifier();
    createLightingModifier();
    createFragmentModifier();

    // Set the lighting model to PBR because we want to take irradiance
    // (PBR ambient light) into account when computing how much to diminish
    // the shadow on the transparent surface. Note that for non-PBR devices
    // this will automatically regress to Phong/Blinn.
    material->setLightingModel(VROLightingModel::PhysicallyBased);
    material->setWritesToDepthBuffer(false);
    material->setCastsShadows(false);

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
            "if (totalShadow != 0.0) {",
            "    highp float ambient_brightness = dot(_ambient.rgb, vec3(0.2126, 0.7152, 0.0722));",
            "    _output_color = vec4(0, 0, 0, max(min(1.0, totalShadow) - ambient_brightness, 0.0));",
            "} else {\n",
            "    _output_color = vec4(0, 0, 0, 0);",
            "}",
        };
        sShadowARFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                       modifierCode);
    }
    return sShadowARFragmentModifier;
}

