//
//  Shaders.metal
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
#include "SharedStructures.h"

using namespace metal;

// Variables in constant address space
constant float3 light_position = float3(0.0, 1.0, -1.0);
constant float4 ambient_color  = float4(0.18, 0.24, 0.8, 1.0);

constexpr sampler s(coord::normalized,
                    address::repeat,
                    filter::linear);

typedef struct {
    float3 position [[ attribute(0) ]];
    float2 uv [[ attribute(1) ]];
    float3 normal [[ attribute(2) ]];
} vertex_t;

typedef struct {
    float4 position [[ position ]];
    float4 color;
    float2 uv;
} ColorInOut;

// Vertex shader function
vertex ColorInOut lighting_vertex(vertex_t vertex_array [[ stage_in ]],
                                  constant uniforms_t& uniforms [[ buffer(1) ]]) {
    ColorInOut out;
    
    float4 in_position = float4(vertex_array.position, 1.0);
    out.position = uniforms.modelview_projection_matrix * in_position;
    out.uv = vertex_array.uv;
    
    float4 eye_normal = normalize(uniforms.normal_matrix * float4(vertex_array.normal, 0.0));
    float n_dot_l = dot(eye_normal.rgb, normalize(light_position));
    n_dot_l = fmax(0.0, n_dot_l);
    
    out.color = uniforms.diffuse_color;
    return out;
}

// Fragment shader function
fragment float4 lighting_fragment(ColorInOut in [[ stage_in ]],
                                  texture2d<float> diffuse_texture [[ texture(0) ]]) {
    return in.color * diffuse_texture.sample(s, in.uv);
}

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
   CONSTANT LIGHTING MODEL
   --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float4 color;
    float2 texcoord;
    float4 ambient_light;
} VROConstantLightingVertexOut;

vertex VROConstantLightingVertexOut constant_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                             constant VROViewUniforms &view [[ buffer(1) ]],
                                                             constant VROConstantLightingUniforms &lighting [[ buffer(2) ]]) {
    VROConstantLightingVertexOut out;
    
    float4 in_position = float4(attributes.position, 1.0);
    out.position = view.modelview_projection_matrix * in_position;
    out.texcoord = attributes.texcoord;
    out.ambient_light = lighting.ambient_light;
    out.color = lighting.ambient_color * lighting.ambient_light + lighting.diffuse_color;
    
    return out;
}

fragment float4 constant_lighting_fragment_cc(VROConstantLightingVertexOut in [[ stage_in ]]) {
    return in.color;
}

fragment float4 constant_lighting_fragment_ct(VROConstantLightingVertexOut in [[ stage_in ]],
                                              texture2d<float> texture [[ texture(0) ]]) {
    return in.color + texture.sample(s, in.texcoord);
}

fragment float4 constant_lighting_fragment_tt(VROConstantLightingVertexOut in [[ stage_in ]],
                                              texture2d<float> ambient_texture [[ texture(0) ]],
                                              texture2d<float> diffuse_texture [[ texture(1) ]]) {
    return ambient_texture.sample(s, in.texcoord) * in.ambient_light + diffuse_texture.sample(s, in.texcoord);
}

/* ---------------------------------------
   PHONG LIGHTING MODEL
   --------------------------------------- */

/* ---------------------------------------
   BLINN LIGHTING MODEL
   --------------------------------------- */

/* ---------------------------------------
   LAMBERT LIGHTING MODEL
   --------------------------------------- */

typedef struct {
    float4 position [[ position ]];
    float4 color;
    float2 texcoord;
    float4 ambient_light;
} VROLambertLightingVertexOut;

vertex VROLambertLightingVertexOut lambert_lighting_vertex(VRORendererAttributes attributes [[ stage_in ]],
                                                            constant VROViewUniforms &view [[ buffer(1) ]],
                                                            constant VROLambertLightingUniforms &lighting [[ buffer(2) ]]) {
    VROLambertLightingVertexOut out;
    
    float4 in_position = float4(attributes.position, 1.0);
    out.position = view.modelview_projection_matrix * in_position;
    out.texcoord = attributes.texcoord;
    
    float4 eye_normal = normalize(view.normal_matrix * float4(attributes.normal, 0.0));
    float n_dot_l = dot(eye_normal.rgb, normalize(light_position));
    n_dot_l = fmax(0.0, n_dot_l);
    
    out.ambient_light = lighting.ambient_light;
    out.color = lighting.ambient_color * lighting.ambient_light + lighting.diffuse_color * n_dot_l;
    return out;
}

fragment float4 lambert_lighting_fragment_cc(VROConstantLightingVertexOut in [[ stage_in ]]) {
    return in.color;
}

fragment float4 lambert_lighting_fragment_ct(VROConstantLightingVertexOut in [[ stage_in ]],
                                             texture2d<float> texture [[ texture(0) ]]) {
    return in.color + texture.sample(s, in.texcoord);
}

fragment float4 lambert_lighting_fragment_tt(VROLambertLightingVertexOut in [[ stage_in ]],
                                             texture2d<float> ambient_texture [[ texture(0) ]],
                                             texture2d<float> diffuse_texture [[ texture(1) ]]) {
    return ambient_texture.sample(s, in.texcoord) * in.ambient_light + diffuse_texture.sample(s, in.texcoord);
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
