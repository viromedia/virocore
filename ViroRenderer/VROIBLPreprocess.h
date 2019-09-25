//
//  VROIBLPreprocess.h
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

#ifndef VROIBLPreprocess_h
#define VROIBLPreprocess_h

#include "VROPreprocess.h"

class VROTexture;
class VROEquirectangularToCubeRenderPass;
class VROIrradianceRenderPass;
class VROPrefilterRenderPass;
class VROBRDFRenderPass;

enum class VROIBLPhase {
    Idle,
    CubeConvert,
    IrradianceConvolution,
    PrefilterConvolution,
    BRDFConvolution
};

class VROIBLPreprocess : public VROPreprocess {
public:
    VROIBLPreprocess();
    virtual ~VROIBLPreprocess();
    
    virtual void execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                         std::shared_ptr<VRODriver> driver);
    
private:
    
    VROIBLPhase _phase;
    std::shared_ptr<VROEquirectangularToCubeRenderPass> _equirectangularToCubePass;
    std::shared_ptr<VROIrradianceRenderPass> _irradiancePass;
    std::shared_ptr<VROPrefilterRenderPass> _prefilterPass;
    std::shared_ptr<VROBRDFRenderPass> _brdfPass;

    std::shared_ptr<VROTexture> _currentLightingEnvironment;
    std::shared_ptr<VROTexture> _cubeLightingEnvironment;
    std::shared_ptr<VROTexture> _irradianceMap;
    std::shared_ptr<VROTexture> _prefilterMap;
    std::shared_ptr<VROTexture> _brdfMap;

    void doCubeConversionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                               std::shared_ptr<VRODriver> driver);
    void doIrradianceConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                      std::shared_ptr<VRODriver> driver);
    void doPrefilterConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                     std::shared_ptr<VRODriver> driver);
    void doBRDFComputationPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                     std::shared_ptr<VRODriver> driver);
};

#endif /* VROIBLPreprocess_h */
