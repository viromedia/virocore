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

static thread_local std::shared_ptr<VROImagePostProcess> sGrayScale;
static thread_local std::shared_ptr<VROImagePostProcess> sSepia;
static thread_local std::shared_ptr<VROImagePostProcess> sSinCity;
static thread_local std::shared_ptr<VROImagePostProcess> sBarallel;
static thread_local std::shared_ptr<VROImagePostProcess> sPincushion;
static thread_local std::shared_ptr<VROImagePostProcess> sToonify;
static thread_local std::shared_ptr<VROImagePostProcess> sInverted;
static thread_local std::shared_ptr<VROImagePostProcess> sThermalVision;
static thread_local std::shared_ptr<VROImagePostProcess> sPixellated;
static thread_local std::shared_ptr<VROImagePostProcess> sCrossHatch;
static thread_local std::shared_ptr<VROImagePostProcess> sEmptyEffect;

VROPostProcessEffectFactory::VROPostProcessEffectFactory() {}
VROPostProcessEffectFactory::~VROPostProcessEffectFactory() {
    sGrayScale = nullptr;
    sSepia = nullptr;
    sSinCity = nullptr;
    sBarallel = nullptr;
    sPincushion = nullptr;
    sToonify = nullptr;
    sInverted = nullptr;
    sThermalVision = nullptr;
    sPixellated = nullptr;
    sCrossHatch = nullptr;
    sEmptyEffect = nullptr;
}

void VROPostProcessEffectFactory::enableEffect(VROPostProcessEffect effect, std::shared_ptr<VRODriver> driver){
    // Check and return if effect has already been applied.
    for (int i = 0; i < _cachedPrograms.size(); i ++) {
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
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createBarallelDistortion(driver));
    } else if (effect == VROPostProcessEffect::PincushionDistortion){
        appliedEffect = std::pair<VROPostProcessEffect, std::shared_ptr<VROImagePostProcess>>(effect, createPinCusionDistortion(driver));
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

std::shared_ptr<VRORenderTarget> VROPostProcessEffectFactory::handlePostProcessing(std::shared_ptr<VRORenderTarget> source,
                                                                                   std::shared_ptr<VRORenderTarget> targetA,
                                                                                   std::shared_ptr<VRORenderTarget> targetB,
                                                                                   std::shared_ptr<VRODriver> driver) {
    if (_cachedPrograms.size() > 0) {
        targetA->hydrate();
        targetB->hydrate();
        return renderEffects(source, targetA, targetB, driver);
    } else {
        return source;
    }
}

std::shared_ptr<VRORenderTarget> VROPostProcessEffectFactory::renderEffects(std::shared_ptr<VRORenderTarget> input,
                                                                            std::shared_ptr<VRORenderTarget> targetA,
                                                                            std::shared_ptr<VRORenderTarget> targetB,
                                                                            std::shared_ptr<VRODriver> driver) {
    // Compound post process effects by blitting ping-pong style between input and output targets.
    std::shared_ptr<VRORenderTarget> outputTarget = input;
    
    for (int i = 0; i < _cachedPrograms.size(); i ++) {
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

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createBarallelDistortion(std::shared_ptr<VRODriver> driver) {
    if (!sBarallel) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp vec2 xy = 2.0 * v_texcoord.xy - 1.0;",
                "highp vec2 uv;",
                "highp float d = length(xy);",
                "if (d < 1.0) {",
                "    highp float BarrelPower = 1.5;",
                "    highp float theta  = atan(xy.y, xy.x);",
                "    highp float radius = length(xy);",
                "    radius = pow(radius, BarrelPower);",
                "    xy.x = radius * cos(theta);",
                "    xy.y = radius * sin(theta);",
                "    uv = 0.5 * (xy + 1.0);",
                " } else {",
                "    uv = v_texcoord.xy;",
                " }",
                " frag_color = texture(source_texture, uv);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sBarallel = driver->newImagePostProcess(shader);
    }
    return sBarallel;
}

std::shared_ptr<VROImagePostProcess> VROPostProcessEffectFactory::createPinCusionDistortion(std::shared_ptr<VRODriver> driver) {
    if (!sPincushion) {
        std::vector<std::string> samplers = { "source_texture" };
        std::vector<std::string> code = {
                "uniform sampler2D source_texture;",
                "highp vec2 xy = 2.0 * v_texcoord.xy - 1.0;",
                "highp vec2 uv;",
                "highp float d = length(xy);",
                "if (d < 1.0) {",
                "    highp float BarrelPower = .5;",
                "    highp float theta  = atan(xy.y, xy.x);",
                "    highp float radius = length(xy);",
                "    radius = pow(radius, BarrelPower);",
                "    xy.x = radius * cos(theta);",
                "    xy.y = radius * sin(theta);",
                "    uv = 0.5 * (xy + 1.0);",
                "} else {",
                "    uv = v_texcoord.xy;",
                "}",
                "frag_color = texture(source_texture, uv);",
        };
        std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
        sPincushion = driver->newImagePostProcess(shader);
    }
    return sPincushion;
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
