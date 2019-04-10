//
//  VROPostProcessEffectFactory.cpp
//  ViroKit
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPostProcessEffectFactory.h"
#include "VROImagePostProcess.h"
#include "VRORenderTarget.h"
#include "VROImageShaderProgram.h"
#include "VRODriver.h"
#include "VROShaderModifier.h"
#include "VROTime.h"

static thread_local std::shared_ptr<VROImagePostProcess> sGrayScale;
static thread_local std::shared_ptr<VROImagePostProcess> sSepia;
static thread_local std::shared_ptr<VROImagePostProcess> sSinCity;
static thread_local std::shared_ptr<VROImagePostProcess> sBarallel;
static thread_local std::shared_ptr<VROImagePostProcess> sToonify;
static thread_local std::shared_ptr<VROImagePostProcess> sInverted;
static thread_local std::shared_ptr<VROImagePostProcess> sThermalVision;
static thread_local std::shared_ptr<VROImagePostProcess> sPixellated;
static thread_local std::shared_ptr<VROImagePostProcess> sCrossHatch;
static thread_local std::shared_ptr<VROImagePostProcess> sSwirl;
static thread_local std::shared_ptr<VROImagePostProcess> sZoomEffect;
static thread_local std::shared_ptr<VROImagePostProcess> sWindowMask;
static thread_local std::shared_ptr<VROImagePostProcess> sEmptyEffect;

VROPostProcessEffectFactory::VROPostProcessEffectFactory() {
    _enabledWindowMask = false;
    _swirlSpeedMultiplier = 2;
}

VROPostProcessEffectFactory::~VROPostProcessEffectFactory() {
    sGrayScale = nullptr;
    sSepia = nullptr;
    sSinCity = nullptr;
    sBarallel = nullptr;
    sToonify = nullptr;
    sInverted = nullptr;
    sThermalVision = nullptr;
    sPixellated = nullptr;
    sCrossHatch = nullptr;
    sSwirl = nullptr;
    sZoomEffect = nullptr;
    sWindowMask = nullptr;
    sEmptyEffect = nullptr;
}

void VROPostProcessEffectFactory::enableEffect(VROPostProcessEffect effect, std::shared_ptr<VRODriver> driver){
    // Check and return if effect has already been applied.
    for (int i = 0; i < _cachedPrograms.size(); i++) {
        if (_cachedPrograms[i].first == effect) {
            pwarn("Effect has already been applied!");
            return;
        }
    }

    // Ensure we have an empty effect to blit the final result through.
    createEmptyEffect(driver);

    std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>> appliedEffect;
    if (effect == VROPostProcessEffect::GrayScale){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createGreyScale(driver));
    } else if (effect == VROPostProcessEffect::Sepia){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createSepia(driver));
    } else if (effect == VROPostProcessEffect::SinCity){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createSinCity(driver));
    } else if (effect == VROPostProcessEffect::BarallelDistortion){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createCircularDistortion(driver, 1.75));
    } else if (effect == VROPostProcessEffect::PincushionDistortion){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createCircularDistortion(driver, 0.5));
    } else if (effect == VROPostProcessEffect::Toonify){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createToonify(driver));
    } else if (effect == VROPostProcessEffect::Inverted){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createInverted(driver));
    } else if (effect == VROPostProcessEffect::ThermalVision){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createThermalVision(driver));
    } else if (effect == VROPostProcessEffect::Pixelated){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createPixel(driver));
    } else if (effect == VROPostProcessEffect::CrossHatch){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createCrossHatch(driver));
    } else if (effect == VROPostProcessEffect::SwirlDistortion){
        _circularDistortion = 0.35;
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createSwirlEffect(driver));
    } else if (effect == VROPostProcessEffect::ZoomInDistortion){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createZoomEffect(driver));
    } else {
        pwarn("Selected unsupported effect!");
        return;
    }
    _cachedPrograms.push_back(appliedEffect);
}

void VROPostProcessEffectFactory::disableEffect(VROPostProcessEffect effect){
    for (std::vector<std::pair<VROPostProcessEffect,
            std::shared_ptr<VROImagePostProcess>>>::iterator it=_cachedPrograms.begin();
         it!=_cachedPrograms.end(); ) {
        std::pair<VROPostProcessEffect,
                std::shared_ptr<VROImagePostProcess>> appliedEffect = *it;
        if (appliedEffect.first == effect) {
            _cachedPrograms.erase(it);
        } else {
            ++it;
        }
    }
}

void VROPostProcessEffectFactory::clearAllEffects(){
    _cachedPrograms.clear();
};

void VROPostProcessEffectFactory::enableWindowMask(std::shared_ptr<VRODriver> driver) {
    if (!sWindowMask) {
        std::vector<std::string> samplers = { "source_texture", "post_processed_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "uniform sampler2D post_processed_texture;",
                "uniform highp vec3 tl;",
                "uniform highp vec3 tr;",
                "uniform highp vec3 bl;",
                "uniform highp vec3 br;",

                // For the given quad that we are drawing, determine if pixel is within window mask or not.
                "highp vec2 p = v_texcoord.xy;",
                "bool isInsideBox = (",
                "        (((tr.x - tl.x) * (p.y - tl.y)) - ((p.x - tl.x) * (tr.y - tl.y)) ) < 0.0",
                "        && ( ((br.x - tr.x) * (p.y - tr.y)) - ((p.x - tr.x) * (br.y - tr.y)) ) < 0.0",
                "        && ( ((bl.x - br.x) * (p.y - br.y)) - ((p.x - br.x) * (bl.y - br.y)) ) < 0.0",
                "        && ( ((tl.x - bl.x) * (p.y - bl.y)) - ((p.x - bl.x) * (tl.y - bl.y)) ) < 0.0",
                ");",
                "frag_color = (isInsideBox) ? texture(post_processed_texture, v_texcoord) : texture(source_texture, v_texcoord);",
        };

        // Add modifiers for blurring the image horizontally and vertically.
        std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
        std::weak_ptr<VROPostProcessEffectFactory> weakSelf = std::dynamic_pointer_cast<VROPostProcessEffectFactory>(shared_from_this());
        modifier->setUniformBinder("tl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           // Modify the equation settings for the inclusive rectangle.
                                           uniform->setVec3({strongSelf->_maskTl.x, strongSelf->_maskTl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("tr", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           // Modify the equation settings for the inclusive rectangle.
                                           uniform->setVec3({strongSelf->_maskTr.x, strongSelf->_maskTr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("bl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           // Modify the equation settings for the inclusive rectangle.
                                           uniform->setVec3({strongSelf->_maskBl.x, strongSelf->_maskBl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("br", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           // Modify the equation settings for the inclusive rectangle.
                                           uniform->setVec3({strongSelf->_maskBr.x, strongSelf->_maskBr.y, 0});
                                       }
                                   });


        // Finally create our ImagePostProcess program and cache it.
        std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
        std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
        sWindowMask = driver->newImagePostProcess(shader);
    }

    _enabledWindowMask = true;
}

void VROPostProcessEffectFactory::disableWindowMask(std::shared_ptr<VRODriver> driver) {
    _enabledWindowMask = false;
}

void VROPostProcessEffectFactory::updateWindowMask(VROVector3f tl, VROVector3f tr, VROVector3f bl, VROVector3f br) {
    _maskTl = tl;
    _maskTr = tr;
    _maskBl = bl;
    _maskBr = br;
}

std::shared_ptr<VRORenderTarget> VROPostProcessEffectFactory::handlePostProcessing(std::shared_ptr<VRORenderTarget> source,
                                                                                   std::shared_ptr<VRORenderTarget> targetA,
                                                                                   std::shared_ptr<VRORenderTarget> targetB,
                                                                                   std::shared_ptr<VRODriver> driver) {
    if (_cachedPrograms.size() == 0) {
        return source;
    }

    // Blit effects as usual and return the post process result.
    targetA->hydrate();
    targetB->hydrate();
    std::shared_ptr<VRORenderTarget> outputTarget = renderEffects(source, targetA, targetB, driver);
    if (!_enabledWindowMask) {
        return outputTarget;
    }

    // If the post process mask is enabled, blend the source and post processed result
    std::shared_ptr<VRORenderTarget> finalOutput = outputTarget == targetA ? targetB : targetA;
    driver->bindRenderTarget(finalOutput, VRORenderTargetUnbindOp::Invalidate);
    sWindowMask->blit({source->getTexture(0), outputTarget->getTexture(0)}, driver);
    return finalOutput;
}

std::shared_ptr<VRORenderTarget> VROPostProcessEffectFactory::renderEffects(std::shared_ptr<VRORenderTarget> input,
                                                                            std::shared_ptr<VRORenderTarget> targetA,
                                                                            std::shared_ptr<VRORenderTarget> targetB,
                                                                            std::shared_ptr<VRODriver> driver) {
    // Save the aspect ratio of the final output if needed.
    _outputAspectRatio.x = targetB->getWidth();
    _outputAspectRatio.y = targetB->getHeight();

    // Compound post process effects by blitting ping-pong style between input and output targets.
    std::shared_ptr<VRORenderTarget> outputTarget = input;
    
    for (int i = 0; i < _cachedPrograms.size(); i++) {
        std::shared_ptr<VROImagePostProcess> &postProcess = _cachedPrograms[i].second;
        
        if (i == 0) {
            driver->bindRenderTarget(targetA, VRORenderTargetUnbindOp::Invalidate);
            postProcess->blit({ input->getTexture(0) }, driver);
            outputTarget = targetA;
        }
        else if (i % 2 == 1) {
            driver->bindRenderTarget(targetB, VRORenderTargetUnbindOp::Invalidate);
            postProcess->blit({ targetA->getTexture(0) }, driver);
            outputTarget = targetB;
        }
        else {
            driver->bindRenderTarget(targetA, VRORenderTargetUnbindOp::Invalidate);
            postProcess->blit({ targetB->getTexture(0) },  driver);
            outputTarget = targetA;
        }
    }
    
    return outputTarget;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createGreyScale(std::shared_ptr<VRODriver> driver) {
    if (!sGrayScale) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "frag_color = texture(source_texture, v_texcoord);",
                "highp float average = 0.2126 * frag_color.r + 0.7152 * frag_color.g + 0.0722 * frag_color.b;",
                "frag_color = vec4(average, average, average, 1.0);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sGrayScale = driver->newImagePostProcess(shader);
    }
    return sGrayScale;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createSepia(std::shared_ptr<VRODriver> driver) {
    if (!sSepia) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp float adjust = 0.9;",
                "highp vec4 color = texture(source_texture, v_texcoord);",
                "highp vec4 outputColor;",
                "outputColor.r = min(1.0,(color.r * (1.0 - (0.607 * adjust))) + (color.g * 0.769 * adjust) + (color.b * 0.189 * adjust));",
                "outputColor.g = min(1.0,(color.r * 0.349 * adjust) + (color.g * (1.0 - (0.314 * adjust))) + (color.b * 0.168 * adjust));",
                "outputColor.b = min(1.0,(color.r * 0.272 * adjust) + (color.g * 0.534 * adjust) + (color.b * (1.0 - (0.869 * adjust))));",
                "frag_color = vec4(outputColor.rgb, 1.0);"
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sSepia = driver->newImagePostProcess(shader);
    }
    return sSepia;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createSinCity(std::shared_ptr<VRODriver> driver) {
    if (!sSinCity) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp vec4 color = texture(source_texture, v_texcoord);",
                "highp float thresh = 0.1f;",
                "highp vec4 lumcoeff = vec4(0.299,0.587,0.114,0.);",
                "highp float luminance = dot(color,lumcoeff);",
                "highp float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;",
                "highp vec4 luma = vec4(average);",
                "highp vec4 sat = vec4((color.r - (luma.r/3.)), (color.g - (luma.g/3.)), (color.b - (luma.b/3.)), 1.0);",
                "highp float mixamount = (thresh >= sat.r || sat.b >= thresh || sat.g >= thresh) ? thresh : sat.r;",
                "frag_color = mix(luma, sat, mixamount);",
        };

        std::vector<std::string> darkerScene = getHBCSModification(0, .45, .55 , .45);
        code.insert(code.end(), darkerScene.begin(), darkerScene.end());

        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sSinCity = driver->newImagePostProcess(shader);
    }
    return sSinCity;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createCircularDistortion(std::shared_ptr<VRODriver> driver,
                                                                                           float distortion) {
    if (!sBarallel) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "uniform highp vec3 tl;",
                "uniform highp vec3 tr;",
                "uniform highp vec3 bl;",
                "uniform highp vec3 br;",
                "uniform highp float distortion;",

                "if (v_texcoord.x < bl.x ||",
                "v_texcoord.x > br.x ||",
                "v_texcoord.y < bl.y ||",
                "v_texcoord.y > tl.y) {",
                "    frag_color = texture(source_texture, v_texcoord);"
                "    return;",
                "}",

                // Now determine the window of the area we wish to blur.
                // Normalize this to [-1, 1] to be procesed.
                "highp float width = tr.x - tl.x;",
                "highp float height = tl.y - bl.y;",
                "highp float shift_x = bl.x;",
                "highp float normalizedX = (( 2.0 * (v_texcoord.x - shift_x) ) - width) / width;",
                "highp float shift_y = bl.y;",
                "highp float normalizedY = (( 2.0 * (v_texcoord.y - shift_y) ) - height) / height;",
                "highp vec2 targetxy = vec2(normalizedX, normalizedY);",
                "highp float d = length(targetxy);",

                // If we are ouside the distortion area, render as normal
                "if (d > 1.0) {",
                "    frag_color = texture(source_texture, v_texcoord);"
                "    return;",
                "}",

                // Else, we are in the fishEye zone
                "highp vec2 uv;",
                "highp float theta  = atan(targetxy.y , targetxy.x);",
                "highp float radius = length(targetxy);",
                "radius = pow(radius, distortion);",
                "uv.x = (radius * cos(theta));",
                "uv.y = (radius * sin(theta));",

                // Undo move back into screen space.
                "uv.x = (((uv.x * width)  + width ) * 0.5) + shift_x;",
                "uv.y = (((uv.y * height) + height ) * 0.5) + shift_y;",

                // Apply the colors.,
                "frag_color = texture(source_texture, uv);"
        };

        // Add modifiers for blurring the image horizontally and vertically.
        std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
        std::weak_ptr<VROPostProcessEffectFactory> weakSelf = std::dynamic_pointer_cast<VROPostProcessEffectFactory>(shared_from_this());
        modifier->setUniformBinder("tl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTl.x, strongSelf->_maskTl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("tr", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTr.x, strongSelf->_maskTr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("bl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBl.x, strongSelf->_maskBl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("br", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBr.x, strongSelf->_maskBr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("distortion", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setFloat(strongSelf->_circularDistortion);
                                       }
                                   });

        // Finally create our ImagePostProcess program and cache it.
        std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
        std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
        sBarallel = driver->newImagePostProcess(shader);
    }
    _circularDistortion = distortion;
    return sBarallel;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createToonify(std::shared_ptr<VRODriver> driver) {
    if (!sToonify) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp float edge_thres = 0.5;",
                "highp float edge_thres2 = 5.0;",
                "highp vec4 color = texture(source_texture, v_texcoord);",
                "highp float dxtex = 1.0 / float(textureSize(source_texture, 0));",
                "highp float dytex = 1.0 / float(textureSize(source_texture, 0));",
                "highp float pix[9];",
                "int k = -1;",
                "highp float delta;",
                "for (int i = -1; i < 2; i++) {",
                "    for (int j = -1; j < 2; j++) {",
                "        k++;",
                "        highp vec4 getPixel = texture(source_texture, v_texcoord + vec2(float(i) * dxtex, float(j) * dytex));",
                "        pix[k] = (getPixel.r + getPixel.g + getPixel.b) / 3.;",
                "    }",
                "}",
                "delta = (abs(pix[1] - pix[7]) + abs(pix[5] - pix[3]) + abs(pix[0] - pix[8]) + abs(pix[2] - pix[6])) / 4.;",
                "highp float edg = clamp(edge_thres2 * delta, 0.0, 1.0);"

                "highp vec3 vRGB = (edg >= edge_thres)? vec3(0.0,0.0,0.0):vec3(color.r,color.g,color.b);",
                "frag_color = vec4(vRGB.x,vRGB.y,vRGB.z, 1);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sToonify = driver->newImagePostProcess(shader);
    }
    return sToonify;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createInverted(std::shared_ptr<VRODriver> driver) {
    if (!sInverted) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp vec4 color = texture(source_texture, v_texcoord);",
                "frag_color = vec4(1.0 - color.rgb, 1.0);"
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sInverted = driver->newImagePostProcess(shader);
    }
    return sInverted;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createThermalVision(std::shared_ptr<VRODriver> driver) {
    if (!sThermalVision) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp vec3 pixcol = texture(source_texture, v_texcoord).rgb;",
                "highp vec3 colors[3];",
                "colors[0] = vec3(0.,0.,1.);",
                "colors[1] = vec3(1.,1.,0.);",
                "colors[2] = vec3(1.,0.,0.);",
                "highp float lum = (pixcol.r+pixcol.g+pixcol.b)/3.;",
                "int ix = (lum < 0.5)? 0:1;",
                "highp vec3 tc = mix(colors[ix],colors[ix+1],(lum-float(ix)*0.5)/0.5);",
                "frag_color = vec4(tc, 1.0);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sThermalVision = driver->newImagePostProcess(shader);
    }
    return sThermalVision;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createPixel(std::shared_ptr<VRODriver> driver) {
    if (!sPixellated) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp float percent = 2./100.;",
                "highp float dx = percent;",
                "highp float dy = percent;",
                "highp vec2 coord = vec2(dx*floor(v_texcoord.x/dx), dy*floor(v_texcoord.y/dy));",
                "highp vec3 tc = texture(source_texture, coord).rgb;",
                "frag_color = vec4(tc, 1.0);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sPixellated = driver->newImagePostProcess(shader);
    }
    return sPixellated;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createCrossHatch(std::shared_ptr<VRODriver> driver) {
    if (!sCrossHatch) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp float hatch_y_offset= 5.0;",
                "highp float lum_threshold_1= 1.0;",
                "highp float lum_threshold_2= 0.7;",
                "highp float lum_threshold_3= 0.5;",
                "highp float lum_threshold_4= 0.3;",
                "highp float lum = length(texture(source_texture, v_texcoord).rgb);",
                "highp vec3 tc = vec3(1.0, 1.0, 1.0);",
                "if (lum < lum_threshold_1 && mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) {",
                "    tc = vec3(0.0, 0.0, 0.0);",
                "}",
                "if (lum < lum_threshold_2 && mod(gl_FragCoord.x - gl_FragCoord.y, 10.0) == 0.0) {",
                "    tc = vec3(0.0, 0.0, 0.0);",
                "}",
                "if (lum < lum_threshold_3 && mod(gl_FragCoord.x + gl_FragCoord.y - hatch_y_offset, 10.0) == 0.0){",
                "    tc = vec3(0.0, 0.0, 0.0);",
                "}",
                "if (lum < lum_threshold_4 && mod(gl_FragCoord.x - gl_FragCoord.y - hatch_y_offset, 10.0) == 0.0){",
                "    tc = vec3(0.0, 0.0, 0.0);",
                "}",
                "frag_color = vec4(tc, 1.0);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sCrossHatch = driver->newImagePostProcess(shader);
    }
    return sCrossHatch;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createSwirlEffect(std::shared_ptr<VRODriver> driver) {
    if (!sSwirl) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "uniform highp vec3 tl;",
                "uniform highp vec3 tr;",
                "uniform highp vec3 bl;",
                "uniform highp vec3 br;",
                "uniform highp float currentTimeSec;",
                "uniform highp vec3 aspectRatio;",
                "uniform highp float distortionPower;",

                "if (v_texcoord.x < bl.x ||",
                "v_texcoord.x > br.x ||",
                "v_texcoord.y < bl.y ||",
                "v_texcoord.y > tl.y) {",
                "    frag_color = texture(source_texture, v_texcoord);"
                        "    return;",
                "}",

                // Determine the center of the swirl
                "highp float centerX = tl.x + (tr.x - tl.x) * 0.5;",
                "highp float centerY = bl.y + (tl.y - bl.y) * 0.5;",
                "highp vec2 center = vec2(centerX, centerY);",

                // Determine the current offset from the center.
                // To determine the size of the circle, account for the aspect ratio
                // TODO: account for horizontal ratios
                "highp float ap = aspectRatio.y / aspectRatio.x;",
                "highp vec2 center_corrected = vec2(center.x, center.y * ap);",
                "highp vec2 v_texcoord_corrected = vec2(v_texcoord.x, v_texcoord.y * ap);",
                "highp float dist = length(v_texcoord_corrected - center_corrected);",

                // Now calculate the rotating swirl angle, use width/2 as the radius
                "highp float width = (tr.x - tl.x) / 2.0;",
                "highp float height = (tl.y - bl.y) / 2.0;",
                "highp float radius = width > height ? height : width;",
                "highp float angle = sin(currentTimeSec);",

                // If we are ouside the distortion area, render as normal
                "if (dist > radius) {",
                "    frag_color = texture(source_texture, v_texcoord);",
                "    return;",
                "}",

                // Else, we are in the distortion zone
                "highp vec2 uv;",
                "highp float percent = (radius - dist) / radius;",
                "highp float theta = percent * percent * angle * 8.0;",
                "highp float s = sin(theta * distortionPower);",
                "highp float c = cos(sin(theta * distortionPower));",

                "highp vec2 tc = v_texcoord;",
                "tc -= center;",
                "tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));",
                "tc += center;",

                // Apply the colors.,
                "frag_color = texture(source_texture, tc);"
        };

        // Add modifiers for blurring the image horizontally and vertically.
        std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
        std::weak_ptr<VROPostProcessEffectFactory> weakSelf = std::dynamic_pointer_cast<VROPostProcessEffectFactory>(shared_from_this());
        modifier->setUniformBinder("tl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTl.x, strongSelf->_maskTl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("tr", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTr.x, strongSelf->_maskTr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("bl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBl.x, strongSelf->_maskBl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("br", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBr.x, strongSelf->_maskBr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("currentTimeSec", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setFloat(VROTimeCurrentSeconds() * strongSelf->_swirlSpeedMultiplier);
                                       }
                                   });
        modifier->setUniformBinder("distortionPower", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setFloat(strongSelf->_circularDistortion);
                                       }
                                   });
        modifier->setUniformBinder("aspectRatio", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setVec3({strongSelf->_outputAspectRatio.x, strongSelf->_outputAspectRatio.y, 0});
                                       }
                                   });

        // Finally create our ImagePostProcess program and cache it.
        std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
        std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
        sSwirl = driver->newImagePostProcess(shader);
    }
    _circularDistortion = 0.35;
    return sSwirl;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createZoomEffect(std::shared_ptr<VRODriver> driver) {
    if (!sZoomEffect) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "uniform highp vec3 tl;",
                "uniform highp vec3 tr;",
                "uniform highp vec3 bl;",
                "uniform highp vec3 br;",
                "uniform highp vec3 aspectRatio;",
                "uniform highp float magnification;",
                "highp float border_thickness = 0.01;",

                // Determine the center and radius of the zoom circle
                "highp float width = (tr.x - tl.x) / 2.0;",
                "highp float height = (tl.y - bl.y) / 2.0;",
                "highp float radius = width > height ? height : width;",
                "radius = radius - (2.0 * border_thickness);",

                "highp float centerX = tl.x + ((tr.x - tl.x) * 0.5);",
                "highp float centerY = bl.y + ((tl.y - bl.y) * 0.5);",
                "highp vec2 center = vec2(centerX, centerY);",

                // To determine the size of the circle, account for the aspect ratio
                // TODO: account for horizontal ratios
                "highp float ap = aspectRatio.y / aspectRatio.x;",
                "highp vec2 center_corrected = vec2(center.x, center.y * ap);",
                "highp vec2 v_texcoord_corrected = vec2(v_texcoord.x, v_texcoord.y * ap);",

                // now determine if we are in the circle or not, if not render as normal.
                "highp float dist = length(v_texcoord_corrected - center_corrected);",
                "if (dist > radius + border_thickness) {",
                "    frag_color = texture(source_texture, v_texcoord);",
                "    return;",
                "} else if (dist > radius) {",
                "    frag_color = vec4(0.1, 0.1, 0.1, 1.0);",
                "    return;",
                "}",

                // Else, we are in the distortion zone
                "highp vec2 uv = center + ( (v_texcoord - center) / magnification);",
                "frag_color = texture(source_texture, uv);"
        };

        // Add modifiers for blurring the image horizontally and vertically.
        std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
        std::weak_ptr<VROPostProcessEffectFactory> weakSelf = std::dynamic_pointer_cast<VROPostProcessEffectFactory>(shared_from_this());
        modifier->setUniformBinder("tl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTl.x, strongSelf->_maskTl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("tr", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 1, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskTr.x, strongSelf->_maskTr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("bl", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({0, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBl.x, strongSelf->_maskBl.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("br", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           if (!strongSelf->_enabledWindowMask) {
                                               uniform->setVec3({1, 0, 0});
                                               return;
                                           }

                                           uniform->setVec3({strongSelf->_maskBr.x, strongSelf->_maskBr.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("aspectRatio", VROShaderProperty::Vec3,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setVec3({strongSelf->_outputAspectRatio.x, strongSelf->_outputAspectRatio.y, 0});
                                       }
                                   });
        modifier->setUniformBinder("magnification", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
                                       std::shared_ptr<VROPostProcessEffectFactory> strongSelf = weakSelf.lock();
                                       if (strongSelf) {
                                           uniform->setFloat(strongSelf->_circularDistortion);
                                       }
                                   });
        // Finally create our ImagePostProcess program and cache it.
        std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
        std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
        sZoomEffect = driver->newImagePostProcess(shader);
    }
    _circularDistortion = 2;
    return sZoomEffect;
}

std::vector<std::string> VROPostProcessEffectFactory::getHBCSModification(float hue, float brightness, float contrast, float saturation){
    std::string strHue = VROStringUtil::toString(hue, 2);
    std::string strBrightness = VROStringUtil::toString(brightness, 2);
    std::string strContrast = VROStringUtil::toString(contrast, 2);
    std::string strSaturation = VROStringUtil::toString(saturation, 2);

    return {
            "highp vec4 hbcs = vec4(" + strHue + "," + strBrightness + "," +  strContrast + " , " + strSaturation + ");",
            "highp float _Hue = 360. * hbcs.r;",
            "highp float _Brightness = hbcs.g * 2. - 1.;",
            "highp float _Contrast = hbcs.b * 2.;",
            "highp float _Saturation = hbcs.a * 2.;",

            "highp vec4 outputColor = frag_color;",
            "highp vec3 aColor = outputColor.rgb;",
            "highp float angle = radians(_Hue);",
            "highp vec3 k = vec3(0.57735, 0.57735, 0.57735);",
            "highp float cosAngle = cos(angle);",
            "outputColor.rgb = aColor * cosAngle + cross(k, aColor) * sin(angle) + k * dot(k, aColor) * (1. - cosAngle);",

            "outputColor.rgb = (outputColor.rgb - 0.5f) * (_Contrast) + 0.5f;",
            "outputColor.rgb = outputColor.rgb + _Brightness;",
            "highp float intensity = dot(outputColor.rgb, vec3(0.22, 0.707, 0.0714));",
            "highp vec3 intens = vec3(intensity, intensity, intensity);",
            "outputColor.rgb = mix(intens, outputColor.rgb, _Saturation);",
            "frag_color = outputColor;"
    };
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createEmptyEffect(std::shared_ptr<VRODriver> driver) {
    if (!sEmptyEffect) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "frag_color =  texture(source_texture, v_texcoord);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sEmptyEffect = driver->newImagePostProcess(shader);
    }
    return sEmptyEffect;
}
