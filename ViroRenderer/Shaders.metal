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
   LIGHT APPLICATION
 --------------------------------------- */

float3 VROApplyLight(constant VROLightUniforms &light,
                     float3 surface_pos,
                     float3 surface_normal,
                     float4 material_color);

float3 VROApplyLight(constant VROLightUniforms &light,
                     float3 surface_pos,
                     float3 surface_normal,
                     float4 material_color) {
    
    float3 surface_to_light;
    float attenuation = 1.0;
    
    // Directional light
    if (light.position.w == 0.0) {
        surface_to_light = normalize(light.position.xyz);
        attenuation = 1.0;
    }
    
    // Point light
    else {
        surface_to_light = normalize(light.position.xyz - surface_pos);
        float distance_to_light = length(light.position.xyz - surface_pos);
        attenuation = 1.0 / (1.0 + light.attenuation_falloff_exp * pow(distance_to_light, 2));
        
        // cone restrictions (affects attenuation)
        //float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLight, normalize(light.coneDirection))));
        //if(lightToSurfaceAngle > light.coneAngle){
        //    attenuation = 0.0;
        //}
    }
    
    // Diffuse
    float diffuse_coeff = fmax(0.0, dot(-surface_normal, surface_to_light));
    float3 diffuse = diffuse_coeff * material_color.rgb * light.color;
    
    // Specular
    /*
    float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0) {
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), materialShininess);
    }
    float3 specular = specularCoefficient * materialSpecularColor * light.intensities;
     */
    
    //return ambient + attenuation * (diffuse + specular);
    
    return attenuation * diffuse;
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
    
    float4 ambient_color;
    float4 material_color;
} VROLambertLightingVertexOut;

vertex VROLambertLightingVertexOut lambert_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                           constant VROViewUniforms &view [[ buffer(1) ]],
                                                           constant VROMaterialUniforms &material [[ buffer(2) ]],
                                                           constant VROSceneLightingUniforms &lighting [[ buffer(3) ]]) {
    VROLambertLightingVertexOut out;
    
    float4 in_position = float4(attributes.position, 1.0);
    out.position = view.modelview_projection_matrix * in_position;
    out.texcoord = attributes.texcoord;
    out.normal = normalize(view.normal_matrix * float4(attributes.normal, 0.0)).xyz;
    out.ambient_color = float4(lighting.ambient_light_color, 1.0) * material.diffuse_surface_color;
    out.material_color = material.diffuse_surface_color;
    
    return out;
}

fragment float4 lambert_lighting_fragment_c(VROLambertLightingVertexOut in [[ stage_in ]],
                                            constant VROSceneLightingUniforms &lighting [[ buffer(1) ]]) {
    float3 color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        color += VROApplyLight(lighting.lights[i],
                               in.position.xyz,
                               in.normal,
                               in.material_color);
    }
    
    return in.ambient_color + float4(color, in.material_color.a);
}

fragment float4 lambert_lighting_fragment_t(VROLambertLightingVertexOut in [[ stage_in ]],
                                            texture2d<float> texture [[ texture(0) ]],
                                            constant VROSceneLightingUniforms &lighting [[ buffer(0) ]]) {
    
    float3 color = float3(0, 0, 0);
    for (int i = 0; i < lighting.num_lights; i++) {
        color += VROApplyLight(lighting.lights[i],
                               in.position.xyz,
                               in.normal,
                               in.material_color);
    }
    
    return in.ambient_color + float4(color, in.material_color.a) * texture.sample(s, in.texcoord);
}

/* ---------------------------------------
   PHONG LIGHTING MODEL
   --------------------------------------- */

/* ---------------------------------------
   BLINN LIGHTING MODEL
   --------------------------------------- */

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
