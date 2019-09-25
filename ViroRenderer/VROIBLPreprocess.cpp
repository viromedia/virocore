//
//  VROIBLPreprocess.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "VROIBLPreprocess.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VRORenderTarget.h"
#include "VROEquirectangularToCubeRenderPass.h"
#include "VROIrradianceRenderPass.h"
#include "VROPrefilterRenderPass.h"
#include "VROBRDFRenderPass.h"

// Set to true to display the generated irradiance map as the background, and to
// deactivate specular IBL
static bool kDebugIrradiance = false;

VROIBLPreprocess::VROIBLPreprocess() {
    _phase = VROIBLPhase::Idle;
    _equirectangularToCubePass = std::make_shared<VROEquirectangularToCubeRenderPass>();
    _irradiancePass = std::make_shared<VROIrradianceRenderPass>();
    _prefilterPass = std::make_shared<VROPrefilterRenderPass>();
    _brdfPass = std::make_shared<VROBRDFRenderPass>();
}

VROIBLPreprocess::~VROIBLPreprocess() {
    
}

void VROIBLPreprocess::execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                               std::shared_ptr<VRODriver> driver) {
 
    if (_phase == VROIBLPhase::Idle) {
        std::shared_ptr<VROPortal> portal = scene->getActivePortal();

        // If idling, check if the lighting environment has changed
        if (portal->getLightingEnvironment() != nullptr && portal->getLightingEnvironment() != _currentLightingEnvironment) {
            pinfo("Lighting environment changed");

            _currentLightingEnvironment = portal->getLightingEnvironment();
            _phase = VROIBLPhase::CubeConvert;
        }
        
        // If an environment map has been removed
        if (portal->getLightingEnvironment() == nullptr && _currentLightingEnvironment != nullptr) {
            pinfo("Lighting environment removed");
            context->setIrradianceMap(nullptr);
            context->setBRDFMap(nullptr);
            context->setPrefilteredMap(nullptr);
            
            _currentLightingEnvironment = nullptr;
        }
    }
    
    else if (_phase == VROIBLPhase::CubeConvert) {
        doCubeConversionPhase(scene, context, driver);
        _phase = VROIBLPhase::IrradianceConvolution;
    }
    
    else if (_phase == VROIBLPhase::IrradianceConvolution) {
        doIrradianceConvolutionPhase(scene, context, driver);
        context->setIrradianceMap(_irradianceMap);
        _phase = VROIBLPhase::PrefilterConvolution;
        
        if (kDebugIrradiance) {
            std::shared_ptr<VROPortal> portal = scene->getActivePortal();
            portal->setBackgroundCube(_irradianceMap);
            _phase = VROIBLPhase::Idle;
        }
    }

    else if (_phase == VROIBLPhase::PrefilterConvolution) {
        doPrefilterConvolutionPhase(scene, context, driver);
        context->setPrefilteredMap(_prefilterMap);
        _phase = VROIBLPhase::BRDFConvolution;
    }

    else if (_phase == VROIBLPhase::BRDFConvolution) {
        doBRDFComputationPhase(scene, context, driver);
        context->setBRDFMap(_brdfMap);
        _phase = VROIBLPhase::Idle;
    }
}

void VROIBLPreprocess::doCubeConversionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                             std::shared_ptr<VRODriver> driver) {
    pinfo("   Converting equirectangular texture to cubemap");
    
    VRORenderPassInputOutput inputs;
    inputs.textures[kEquirectangularToCubeHDRTextureInput] = _currentLightingEnvironment;
    _equirectangularToCubePass->render(scene, nullptr, inputs, context, driver);
    _cubeLightingEnvironment = inputs.outputTarget->getTexture(0);
}

void VROIBLPreprocess::doIrradianceConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                                    std::shared_ptr<VRODriver> driver) {
    pinfo("   Convoluting texture to create irradiance map");
    
    VRORenderPassInputOutput inputs;
    inputs.textures[kIrradianceLightingEnvironmentInput] = _cubeLightingEnvironment;
    _irradiancePass->render(scene, nullptr, inputs, context, driver);
    _irradianceMap = inputs.outputTarget->getTexture(0);
}

void VROIBLPreprocess::doPrefilterConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                                   std::shared_ptr<VRODriver> driver) {
    pinfo("   Convoluting texture to create prefiltered map");

    VRORenderPassInputOutput inputs;
    inputs.textures[kPrefilterLightingEnvironmentInput] = _cubeLightingEnvironment;
    _prefilterPass->render(scene, nullptr, inputs, context, driver);
    _prefilterMap = inputs.outputTarget->getTexture(0);
}

void VROIBLPreprocess::doBRDFComputationPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                              std::shared_ptr<VRODriver> driver) {
    pinfo("   Convoluting texture to create BRDF map");

    VRORenderPassInputOutput inputs;
    _brdfPass->render(scene, nullptr, inputs, context, driver);
    _brdfMap = inputs.outputTarget->getTexture(0);
}
