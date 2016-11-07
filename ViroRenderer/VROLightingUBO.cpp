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

void VROLightingUBO::bind(std::shared_ptr<VROShaderProgram> &program) {
    if (program->hasLightingBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getLightingBlockIndex(), _lightingUBOBindingPoint);
    }
}

void VROLightingUBO::writeLights(const std::vector<std::shared_ptr<VROLight>> &lights) {
    pglpush("Lights");
    VROVector3f ambientLight;
    
    VROLightingData data;
    data.num_lights = 0;
    
    for (const std::shared_ptr<VROLight> &light : lights) {
        // Ambient lights have no diffuse color; instead they are added
        // to the aggregate ambient light color, which is passed as a single
        // value into the shader
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor();
        }
        else {
            int index = data.num_lights;
            
            data.lights[index].type = (int) light->getType();
            light->getTransformedPosition().toArray(data.lights[index].position);
            light->getDirection().toArray(data.lights[index].direction);
            light->getColor().toArray(data.lights[index].color);
            data.lights[index].attenuation_start_distance = light->getAttenuationStartDistance();
            data.lights[index].attenuation_end_distance = light->getAttenuationEndDistance();
            data.lights[index].attenuation_falloff_exp = light->getAttenuationFalloffExponent();
            data.lights[index].spot_inner_angle = degrees_to_radians(light->getSpotInnerAngle());
            data.lights[index].spot_outer_angle = degrees_to_radians(light->getSpotOuterAngle());
            
            data.num_lights++;
        }
        
        // Mark all light updated flags false now that the new UBO is being written
        light->setIsUpdated(false);
    }
    
    ambientLight.toArray(data.ambient_light_color);
    
    glBindBuffer(GL_UNIFORM_BUFFER, _lightingUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROLightingData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    pglpop();
}
