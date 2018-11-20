//
//  VRODriverOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
#include "VRODriverOpenGL.h"

// Note this class has limited functionality (most is in the header) but we still require a
// cpp file in order to have a 'key function' which guarantees we get a strong global symbol
// for this class in the typeinfo of the library, so that dynamic_cast works across dlopen
// boundaries.
//
// See here: https://github.com/android-ndk/ndk/issues/533#issuecomment-335977747
VRODriverOpenGL::VRODriverOpenGL() :
        _gpuType(VROGPUType::Normal),
        _lastPurgeFrame(0),
        _softwareGammaPass(false),
        _depthWritingEnabled(true),
        _depthReadingEnabled(true),
        _materialColorWritingMask(VROColorMaskAll),
        _renderTargetColorWritingMask(VROColorMaskAll),
        _aggregateColorWritingMask(VROColorMaskAll),
        _cullMode(VROCullMode::None),
        _blendMode(VROBlendMode::Alpha) {

    _shaderFactory = std::unique_ptr<VROShaderFactory>(new VROShaderFactory());
    _scheduler = std::make_shared<VROFrameScheduler>();
}

VRODriverOpenGL::~VRODriverOpenGL() {

}