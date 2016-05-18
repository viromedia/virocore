#version 300 es
#include constant_functions_fsh

uniform lowp vec3 ambient_light_color;
uniform lowp vec4 material_diffuse_surface_color;
uniform lowp float material_diffuse_intensity;
uniform lowp float material_alpha;

uniform VROSceneLightingUniforms lighting;

in lowp vec3 v_normal;
in highp vec2 v_texcoord;
in highp vec3 v_surface_position;

out lowp vec4 frag_color;

void main() {
    lowp vec3 ambient_color = ambient_light_color * material_diffuse_surface_color.xyz;
    frag_color = vec4(ambient_color, material_alpha);
}