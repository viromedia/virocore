//
//  VROLightingUBO.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/11/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROLightingUBO.h"
#include "VROLight.h"
#include "VROLog.h"
#include "VROMath.h"
#include "VROShaderProgram.h"
#include "VRODriverOpenGL.h"

VROLightingUBO::VROLightingUBO(int hash, const std::vector<std::shared_ptr<VROLight>> &lights,
                               std::shared_ptr<VRODriverOpenGL> driver) :
    _hash(hash),
    _lights(lights),
    _driver(driver),
    _needsFragmentUpdate(false),
    _needsVertexUpdate(false) {

    // Initialize the fragment VBO
    glGenBuffers(1, &_lightingFragmentUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingFragmentUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingFragmentData), NULL, GL_DYNAMIC_DRAW);
            
    // Initialize the vertex VBO
    glGenBuffers(1, &_lightingVertexUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingVertexUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingVertexData), NULL, GL_DYNAMIC_DRAW);
        
    updateLightsFragment();
    updateLightsVertex();
}

VROLightingUBO::~VROLightingUBO() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (driver) {
        glDeleteBuffers(1, &_lightingFragmentUBO);        
        glDeleteBuffers(1, &_lightingVertexUBO);
    }
}

void VROLightingUBO::bind() {
    if (_needsFragmentUpdate) {
        updateLightsFragment();
    }
    else {
        glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sLightingFragmentUBOBindingPoint, _lightingFragmentUBO);
    }
        
    if (_needsVertexUpdate) {
        updateLightsVertex();
    }
    else {
        glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sLightingVertexUBOBindingPoint, _lightingVertexUBO);
    }
}

void VROLightingUBO::updateLightsFragment() {
    pglpush("Lights [Fragment]");
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    VROVector3f ambientLight;
    
    VROLightingFragmentData data;
    data.num_lights = 0;

    for (const std::shared_ptr<VROLight> &light : _lights) {
        VROVector3f lightColor = light->getColor();
        if (driver && driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear) {
            lightColor = VROMathConvertSRGBToLinearColor(lightColor);
        }
        VROVector3f color = lightColor * light->getColorFromTemperature();
        
        // Ambient lights have no diffuse color; instead they are added
        // to the aggregate ambient light color, which is passed as a single
        // value into the shader. They are *always* scaled by intensity / 1000.0,
        // even when running PBR (since they have no physical counterpart)
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += color.scale(light->getIntensity() / 1000.0);
        }
        else {
            int index = data.num_lights;
            
            data.lights[index].type = (int) light->getType();
            data.lights[index].shadow_map_index = light->getShadowMapIndex();
            data.lights[index].shadow_bias = light->getShadowBias();
            data.lights[index].shadow_opacity = light->getShadowOpacity();
            light->getTransformedPosition().toArray(data.lights[index].position);
            light->getTransformedDirection().toArray(data.lights[index].direction);
            color.toArray(data.lights[index].color);
            data.lights[index].intensity = light->getIntensity();
            data.lights[index].attenuation_start_distance = light->getAttenuationStartDistance();
            data.lights[index].attenuation_end_distance = light->getAttenuationEndDistance();
            data.lights[index].attenuation_falloff_exp = light->getAttenuationFalloffExponent();
            data.lights[index].spot_inner_angle = cos(degrees_to_radians(light->getSpotInnerAngle() * 0.5));
            data.lights[index].spot_outer_angle = cos(degrees_to_radians(light->getSpotOuterAngle() * 0.5));
            
            data.num_lights++;
            if (data.num_lights >= kMaxLights) {
                break;
            }
        }
    }
    
    ambientLight.toArray(data.ambient_light_color);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sLightingFragmentUBOBindingPoint, _lightingFragmentUBO);
#if VRO_AVOID_BUFFER_SUB_DATA
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingFragmentData), &data, GL_DYNAMIC_DRAW);
#else
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROLightingFragmentData), &data);
#endif
    
    pglpop();
    _needsFragmentUpdate = false;
}

void VROLightingUBO::updateLightsVertex() {
    pglpush("Lights [Vertex]");
    
    VROLightingVertexData vertexData;
    vertexData.num_lights = 0;
    
    for (const std::shared_ptr<VROLight> &light : _lights) {
        if (light->getType() == VROLightType::Ambient) {
            continue;
        }
        int i = vertexData.num_lights;
        
        const VROMatrix4f &shadowView = light->getShadowViewMatrix();
        memcpy(&vertexData.shadow_view_matrices[i * kFloatsPerMatrix], shadowView.getArray(), kFloatsPerMatrix * sizeof(float));
        const VROMatrix4f &shadowProjection = light->getShadowProjectionMatrix();
        memcpy(&vertexData.shadow_projection_matrices[i * kFloatsPerMatrix], shadowProjection.getArray(), kFloatsPerMatrix * sizeof(float));
        
        vertexData.num_lights++;
        if (vertexData.num_lights >= kMaxLights) {
            break;
        }
    }
    
    glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sLightingVertexUBOBindingPoint, _lightingVertexUBO);
#if VRO_AVOID_BUFFER_SUB_DATA
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingVertexData), &vertexData, GL_DYNAMIC_DRAW);
#else
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROLightingVertexData), &vertexData);
#endif

    pglpop();
    _needsVertexUpdate = false;
}
