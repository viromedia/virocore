//
//  Shaders.metal
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
#include "VROSharedStructures.h"

using namespace metal;

constexpr sampler s(coord::normalized,
                    address::repeat,
                    filter::linear);

/* ---------------------------------------
   GEOMETRY ATTRIBUTES
 --------------------------------------- */

typedef struct {
    float3 position       [[ attribute(0) ]];
    float3 normal         [[ attribute(1) ]];
    float4 color          [[ attribute(2) ]];
    float2 texcoord       [[ attribute(3) ]];
} VRORendererAttributes;

/* ---------------------------------------
   SHARED LIGHTING FUNCTIONS
 --------------------------------------- */

float compute_attenuation(constant VROLightUniforms &light,
                          float3 surface_pos,
                          thread float3 *surface_to_light);

float compute_attenuation(constant VROLightUniforms &light,
                          float3 surface_pos,
                          thread float3 *surface_to_light) {
    
    float attenuation = 1.0;
    
    // Directional light
    if (light.type == 1) {
        *surface_to_light = normalize(light.direction);
        attenuation = 1.0;
    }
    
    // Omni + Spot lights
    else {
        *surface_to_light = -normalize(light.position.xyz - surface_pos);
        float distance_to_light = length(light.position.xyz - surface_pos);
        float d = clamp((distance_to_light - light.attenuation_start_distance) /
                        (light.attenuation_end_distance - light.attenuation_start_distance),
                        0.0, 1.0);
        
        attenuation = 1.0 - pow(d, 1.0 / light.attenuation_falloff_exp);
        
        // Spot light
        if (light.type == 3) {
            float light_surface_angle = acos(dot(*surface_to_light, normalize(light.direction)));
            if (light_surface_angle > light.spot_inner_angle) {
                float t = (light_surface_angle - light.spot_inner_angle) / light.spot_outer_angle;
                attenuation = mix(attenuation, 0.0, t);
            }
        }
    }
    
    return attenuation;
}

/* ---------------------------------------
   CONSTANT LIGHTING MODEL
   --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float4 color;
    float2 texcoord;
    
    float4 ambient_color;
    float4 material_color;
} VROConstantLightingVertexOut;

vertex VROConstantLightingVertexOut constant_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                             constant VROViewUniforms &view [[ buffer(1) ]],
                                                             constant VROMaterialUniforms &material [[ buffer(2) ]],
                                                             constant VROSceneLightingUniforms &lighting [[ buffer(3) ]]) {
    VROConstantLightingVertexOut out;
    
    float4 in_position = float4(attributes.position, 1.0);
    out.position = view.modelview_projection_matrix * in_position;
    out.texcoord = attributes.texcoord;
    out.ambient_color = float4(lighting.ambient_light_color, 1.0) * material.diffuse_surface_color;
    out.material_color = material.diffuse_surface_color;
    
    return out;
}

fragment float4 constant_lighting_fragment_c(VROConstantLightingVertexOut in [[ stage_in ]]) {
    return in.ambient_color;
}

fragment float4 constant_lighting_fragment_t(VROConstantLightingVertexOut in [[ stage_in ]],
                                              texture2d<float> texture [[ texture(0) ]]) {
    return in.ambient_color + float4(in.material_color) * texture.sample(s, in.texcoord);
}

/* ---------------------------------------
 LAMBERT LIGHTING MODEL
 --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float3 normal;
    float2 texcoord;
    
    float3 surface_position;
    
    float4 ambient_color;
    float4 material_color;
} VROLambertLightingVertexOut;

float3 apply_light_lambert(constant VROLightUniforms &light,
                           float3 surface_pos,
                           float3 surface_normal,
                           float4 material_color);

float3 apply_light_lambert(constant VROLightUniforms &light,
                           float3 surface_pos,
                           float3 surface_normal,
                           float4 material_color) {
    
    thread float3 surface_to_light;
    float attenuation = compute_attenuation(light, surface_pos, &surface_to_light);
    
    float diffuse_coeff = fmax(0.0, dot(-surface_normal, surface_to_light));
    return attenuation * diffuse_coeff * material_color.rgb * light.color;
}

vertex VROLambertLightingVertexOut lambert_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                           constant VROViewUniforms &view [[ buffer(1) ]],
                                                           constant VROMaterialUniforms &material [[ buffer(2) ]],
                                                           constant VROSceneLightingUniforms &lighting [[ buffer(3) ]]) {
    VROLambertLightingVertexOut out;
    
    out.position = view.modelview_projection_matrix * float4(attributes.position, 1.0);
    out.texcoord = attributes.texcoord;
    
    out.surface_position = (view.model_matrix * float4(attributes.position, 1.0)).xyz;
    out.normal = normalize(view.normal_matrix * float4(attributes.normal, 0.0)).xyz;

    out.ambient_color = float4(lighting.ambient_light_color, 1.0) * material.diffuse_surface_color;
    out.material_color = material.diffuse_surface_color;
    
    return out;
}

fragment float4 lambert_lighting_fragment_c(VROLambertLightingVertexOut in [[ stage_in ]],
                                            constant VROSceneLightingUniforms &lighting [[ buffer(1) ]]) {
    
    float4 material_diffuse_color = in.material_color;

    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_lambert(lighting.lights[i],
                                                      in.surface_position,
                                                      in.normal,
                                                      material_diffuse_color);
    }
    
    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

fragment float4 lambert_lighting_fragment_t(VROLambertLightingVertexOut in [[ stage_in ]],
                                            texture2d<float> texture [[ texture(0) ]],
                                            constant VROSceneLightingUniforms &lighting [[ buffer(0) ]]) {
    
    float4 material_diffuse_color = texture.sample(s, in.texcoord);
    
    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_lambert(lighting.lights[i],
                                                      in.surface_position,
                                                      in.normal,
                                                      material_diffuse_color);
    }

    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

/* ---------------------------------------
   PHONG LIGHTING MODEL
   --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float3 normal;
    float2 texcoord;
    
    float3 surface_position;
    float3 camera_position;
    
    float4 ambient_color;
    float4 material_color;
    float  material_shininess;
} VROPhongLightingVertexOut;

float3 apply_light_phong(constant VROLightUniforms &light,
                         float3 surface_pos,
                         float3 surface_normal,
                         float3 surface_to_camera,
                         float4 material_diffuse_color,
                         float4 material_specular_color,
                         float  material_shininess);
    
float3 apply_light_phong(constant VROLightUniforms &light,
                         float3 surface_pos,
                         float3 surface_normal,
                         float3 surface_to_camera,
                         float4 material_diffuse_color,
                         float4 material_specular_color,
                         float  material_shininess) {
    
    thread float3 surface_to_light;
    float attenuation = compute_attenuation(light, surface_pos, &surface_to_light);
    
    // Diffuse
    float diffuse_coeff = fmax(0.0, dot(-surface_normal, surface_to_light));
    float3 diffuse = diffuse_coeff * material_diffuse_color.rgb * light.color;
    
    // Specular
    float specular_coeff = 0.0;
    if (diffuse_coeff > 0.0) {
        specular_coeff = pow(max(0.0, dot(surface_to_camera,
                                          reflect(surface_to_light, -surface_normal))),
                             material_shininess);
    }

    float3 specular = specular_coeff * material_specular_color.rgb * light.color;
    return attenuation * (diffuse + specular);
}

vertex VROPhongLightingVertexOut phong_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                       constant VROViewUniforms &view [[ buffer(1) ]],
                                                       constant VROMaterialUniforms &material [[ buffer(2) ]],
                                                       constant VROSceneLightingUniforms &lighting [[ buffer(3) ]]) {
    VROPhongLightingVertexOut out;
    
    out.position = view.modelview_projection_matrix * float4(attributes.position, 1.0);
    out.texcoord = attributes.texcoord;

    out.surface_position = (view.model_matrix * float4(attributes.position, 1.0)).xyz;
    out.camera_position  = view.camera_position;
    out.normal = normalize(view.normal_matrix * float4(attributes.normal, 0.0)).xyz;
    
    out.ambient_color = float4(lighting.ambient_light_color, 1.0) * material.diffuse_surface_color;
    out.material_color = material.diffuse_surface_color;
    out.material_shininess = material.shininess;
    
    return out;
}

fragment float4 phong_lighting_fragment_c(VROPhongLightingVertexOut in [[ stage_in ]],
                                          texture2d<float> specular_texture [[ texture(0) ]],
                                          constant VROSceneLightingUniforms &lighting [[ buffer(1) ]]) {
    
    float4 material_diffuse_color = in.material_color;
    float4 material_specular_color = specular_texture.sample(s, in.texcoord);
    float3 surface_to_camera = normalize(in.camera_position - in.surface_position);

    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_phong(lighting.lights[i],
                                                    in.surface_position,
                                                    in.normal,
                                                    surface_to_camera,
                                                    material_diffuse_color,
                                                    material_specular_color,
                                                    in.material_shininess);
    }
    
    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

fragment float4 phong_lighting_fragment_t(VROPhongLightingVertexOut in [[ stage_in ]],
                                          texture2d<float> diffuse_texture [[ texture(0) ]],
                                          texture2d<float> specular_texture [[ texture(1) ]],
                                          constant VROSceneLightingUniforms &lighting [[ buffer(0) ]]) {
    
    float4 material_diffuse_color  = diffuse_texture.sample(s, in.texcoord);
    float4 material_specular_color = specular_texture.sample(s, in.texcoord);
    float3 surface_to_camera = normalize(in.camera_position - in.surface_position);
    
    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_phong(lighting.lights[i],
                                                    in.surface_position,
                                                    in.normal,
                                                    surface_to_camera,
                                                    material_diffuse_color,
                                                    material_specular_color,
                                                    in.material_shininess);
    }
    
    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

/* ---------------------------------------
   BLINN LIGHTING MODEL
   --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float3 normal;
    float2 texcoord;
    
    float3 surface_position;
    float3 camera_position;
    
    float4 ambient_color;
    float4 material_color;
    float  material_shininess;
} VROBlinnLightingVertexOut;

float3 apply_light_blinn(constant VROLightUniforms &light,
                         float3 surface_pos,
                         float3 surface_normal,
                         float3 surface_to_camera,
                         float4 material_diffuse_color,
                         float4 material_specular_color,
                         float  material_shininess);

float3 apply_light_blinn(constant VROLightUniforms &light,
                         float3 surface_pos,
                         float3 surface_normal,
                         float3 surface_to_camera,
                         float4 material_diffuse_color,
                         float4 material_specular_color,
                         float  material_shininess) {
    
    thread float3 surface_to_light;
    float attenuation = compute_attenuation(light, surface_pos, &surface_to_light);
    
    // Diffuse
    float diffuse_coeff = fmax(0.0, dot(-surface_normal, surface_to_light));
    float3 diffuse = diffuse_coeff * material_diffuse_color.rgb * light.color;
    
    // Specular
    float specular_coeff = 0.0;
    if (diffuse_coeff > 0.0) {
        specular_coeff = pow(max(0.0, dot(normalize(surface_to_camera + surface_to_light),
                                          -surface_normal)),
                             material_shininess);
    }
    
    float3 specular = specular_coeff * material_specular_color.rgb * light.color;
    return attenuation * (diffuse + specular);
}

vertex VROBlinnLightingVertexOut blinn_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                       constant VROViewUniforms &view [[ buffer(1) ]],
                                                       constant VROMaterialUniforms &material [[ buffer(2) ]],
                                                       constant VROSceneLightingUniforms &lighting [[ buffer(3) ]]) {
    VROBlinnLightingVertexOut out;
    
    out.position = view.modelview_projection_matrix * float4(attributes.position, 1.0);
    out.texcoord = attributes.texcoord;
    
    out.surface_position = (view.model_matrix * float4(attributes.position, 1.0)).xyz;
    out.camera_position  = view.camera_position;
    out.normal = normalize(view.normal_matrix * float4(attributes.normal, 0.0)).xyz;
    
    out.ambient_color = float4(lighting.ambient_light_color, 1.0) * material.diffuse_surface_color;
    out.material_color = material.diffuse_surface_color;
    out.material_shininess = material.shininess;
    
    return out;
}

fragment float4 blinn_lighting_fragment_c(VROBlinnLightingVertexOut in [[ stage_in ]],
                                          texture2d<float> specular_texture [[ texture(0) ]],
                                          constant VROSceneLightingUniforms &lighting [[ buffer(1) ]]) {
    
    float4 material_diffuse_color = in.material_color;
    float4 material_specular_color = specular_texture.sample(s, in.texcoord);
    float3 surface_to_camera = normalize(in.camera_position - in.surface_position);
    
    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_phong(lighting.lights[i],
                                                    in.surface_position,
                                                    in.normal,
                                                    surface_to_camera,
                                                    material_diffuse_color,
                                                    material_specular_color,
                                                    in.material_shininess);
    }
    
    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

fragment float4 blinn_lighting_fragment_t(VROBlinnLightingVertexOut in [[ stage_in ]],
                                          texture2d<float> diffuse_texture [[ texture(0) ]],
                                          texture2d<float> specular_texture [[ texture(1) ]],
                                          constant VROSceneLightingUniforms &lighting [[ buffer(0) ]]) {
    
    float4 material_diffuse_color  = diffuse_texture.sample(s, in.texcoord);
    float4 material_specular_color = specular_texture.sample(s, in.texcoord);
    float3 surface_to_camera = normalize(in.camera_position - in.surface_position);
    
    float3 aggregated_light_color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        aggregated_light_color += apply_light_phong(lighting.lights[i],
                                                    in.surface_position,
                                                    in.normal,
                                                    surface_to_camera,
                                                    material_diffuse_color,
                                                    material_specular_color,
                                                    in.material_shininess);
    }
    
    return in.ambient_color + float4(aggregated_light_color, material_diffuse_color.a);
}

/* ---------------------------------------
   DISTORTION SHADERS
   --------------------------------------- */

typedef struct {
    float2 position       [[ attribute(0) ]];
    float  vignette       [[ attribute(1) ]];
    float2 red_texcoord   [[ attribute(2) ]];
    float2 green_texcoord [[ attribute(3) ]];
    float2 blue_texcoord  [[ attribute(4) ]];
} VRODistortionAttributes;

typedef struct {
    float4 position [[ position ]];
    float2 texcoord;
    float  vignette;
} VRODistortionVertexOut;

vertex VRODistortionVertexOut distortion_vertex(VRODistortionAttributes attributes [[ stage_in ]],
                                                constant VRODistortionUniforms &uniforms [[ buffer(1) ]]) {
    
    VRODistortionVertexOut out;
    out.position = float4(attributes.position, 0.0, 1.0);
    out.texcoord = attributes.blue_texcoord.xy * uniforms.texcoord_scale;
    out.vignette = attributes.vignette;
    
    return out;
}

fragment float4 distortion_fragment(VRODistortionVertexOut in [[ stage_in ]],
                                    texture2d<float> texture [[ texture(0) ]]) {
    
    return in.vignette * texture.sample(s, in.texcoord);
}

typedef struct {
    float4 position [[ position ]];
    float2 red_texcoord;
    float2 blue_texcoord;
    float2 green_texcoord;
    float  vignette;
} VRODistortionAberrationVertexOut;

vertex VRODistortionAberrationVertexOut distortion_aberration_vertex(VRODistortionAttributes attributes [[ stage_in ]],
                                                                     constant VRODistortionUniforms &uniforms [[ buffer(1) ]]) {
    
    VRODistortionAberrationVertexOut out;
    out.position = float4(attributes.position, 0.0, 1.0);
    out.red_texcoord = attributes.red_texcoord.xy * uniforms.texcoord_scale;
    out.green_texcoord = attributes.green_texcoord.xy * uniforms.texcoord_scale;
    out.blue_texcoord = attributes.blue_texcoord.xy * uniforms.texcoord_scale;
    out.vignette = attributes.vignette;
    
    return out;
}

fragment float4 distortion_aberration_fragment(VRODistortionAberrationVertexOut in [[ stage_in ]],
                                               texture2d<float> texture [[ texture(0) ]]) {
    
    return in.vignette * float4(texture.sample(s, in.red_texcoord).r,
                                texture.sample(s, in.green_texcoord).g,
                                texture.sample(s, in.blue_texcoord).b,
                                1.0);
}
