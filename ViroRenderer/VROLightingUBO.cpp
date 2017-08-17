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
    _lightingFragmentUBOBindingPoint = driver->generateBindingPoint();
    glGenBuffers(1, &_lightingFragmentUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingFragmentUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingFragmentData), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // Links the UBO and the binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, _lightingFragmentUBOBindingPoint, _lightingFragmentUBO);
        
    // Initialize the vertex VBO
    _lightingVertexUBOBindingPoint = driver->generateBindingPoint();
    glGenBuffers(1, &_lightingVertexUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingVertexUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingVertexData), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, _lightingVertexUBOBindingPoint, _lightingVertexUBO);
        
    updateLightsFragment();
    updateLightsVertex();
}

VROLightingUBO::~VROLightingUBO() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (driver) {
        driver->internBindingPoint(_lightingFragmentUBOBindingPoint);
        glDeleteBuffers(1, &_lightingFragmentUBO);
        
        driver->internBindingPoint(_lightingVertexUBOBindingPoint);
        glDeleteBuffers(1, &_lightingVertexUBO);
    }
}

void VROLightingUBO::unbind(std::shared_ptr<VROShaderProgram> &program) {
    if (program->hasLightingFragmentBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getLightingFragmentBlockIndex(), 0);
    }
    if (program->hasLightingVertexBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getLightingVertexBlockIndex(), 0);
    }
}

void VROLightingUBO::bind(std::shared_ptr<VROShaderProgram> &program) {
    if (_needsFragmentUpdate) {
        updateLightsFragment();
    }
    if (_needsVertexUpdate) {
        updateLightsVertex();
    }
    if (program->hasLightingFragmentBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getLightingFragmentBlockIndex(), _lightingFragmentUBOBindingPoint);
    }
    if (program->hasLightingVertexBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getLightingVertexBlockIndex(), _lightingVertexUBOBindingPoint);
    }
}

void VROLightingUBO::updateLightsFragment() {
    pglpush("Lights [Fragment]");
    VROVector3f ambientLight;
    
    VROLightingFragmentData data;
    data.num_lights = 0;
    
    for (const std::shared_ptr<VROLight> &light : _lights) {
        // Ambient lights have no diffuse color; instead they are added
        // to the aggregate ambient light color, which is passed as a single
        // value into the shader
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor().scale(light->getIntensity() / 1000.0);
        }
        else {
            int index = data.num_lights;
            
            data.lights[index].type = (int) light->getType();
            data.lights[index].shadow_map_index = light->getShadowMapIndex();
            light->getTransformedPosition().toArray(data.lights[index].position);
            light->getDirection().toArray(data.lights[index].direction);
            light->getColor().scale(light->getIntensity() / 1000.0).toArray(data.lights[index].color);
            data.lights[index].attenuation_start_distance = light->getAttenuationStartDistance();
            data.lights[index].attenuation_end_distance = light->getAttenuationEndDistance();
            data.lights[index].attenuation_falloff_exp = light->getAttenuationFalloffExponent();
            data.lights[index].spot_inner_angle = degrees_to_radians(light->getSpotInnerAngle());
            data.lights[index].spot_outer_angle = degrees_to_radians(light->getSpotOuterAngle());
            
            data.num_lights++;
        }
    }
    
    ambientLight.toArray(data.ambient_light_color);
    
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingFragmentUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROLightingFragmentData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
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
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingVertexUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROLightingVertexData), &vertexData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    pglpop();
    _needsVertexUpdate = false;
}
