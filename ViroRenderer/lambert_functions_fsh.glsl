#include lighting_general_functions_fsh

struct VROLambertLighting {
    highp vec4 position;
    lowp vec3 normal;
    highp vec2 texcoord;
    
    highp vec3 surface_position;
    highp vec3 camera_position;
    
    lowp vec3  ambient_color;
    lowp vec4  material_color;
    lowp float diffuse_intensity;
    lowp float material_alpha;
};

lowp vec3 apply_light_lambert(const VROLightUniforms light,
                              highp vec3 surface_pos,
                              lowp vec3 surface_normal,
                              lowp vec4 material_color) {
    
    highp vec3 surface_to_light;
    highp float attenuation = compute_attenuation(light, surface_pos, surface_to_light);
    
    highp float diffuse_coeff = max(0.0, dot(-surface_normal, surface_to_light));
    return attenuation * diffuse_coeff * material_color.rgb * light.color;
}

lowp vec4 lambert_lighting_diffuse_fixed(VROLambertLighting lambert,
                                         const VROSceneLightingUniforms lighting) {
    lowp vec4 material_diffuse_color = lambert.material_color * lambert.diffuse_intensity;
    
    lowp vec3 aggregated_light_color = vec3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_lambert(lighting.lights[i],
                                                      lambert.surface_position,
                                                      lambert.normal,
                                                      material_diffuse_color);
    }
    
    return vec4(lambert.ambient_color + aggregated_light_color,
                lambert.material_alpha * material_diffuse_color.a);
}

lowp vec4 lambert_lighting_diffuse_texture(VROLambertLighting lambert,
                                           sampler2D sampler,
                                           const VROSceneLightingUniforms lighting) {
    
    lowp vec4 material_diffuse_color = texture(sampler, lambert.texcoord) * lambert.diffuse_intensity;
    
    lowp vec3 aggregated_light_color = vec3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_lambert(lighting.lights[i],
                                                      lambert.surface_position,
                                                      lambert.normal,
                                                      material_diffuse_color);
    }
    
    return vec4(lambert.ambient_color + aggregated_light_color,
                lambert.material_alpha * material_diffuse_color.a);
}
