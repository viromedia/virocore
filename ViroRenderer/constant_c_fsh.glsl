#version 300 es
#include constant_functions_fsh

uniform lowp vec4 material_diffuse_surface_color;
uniform lowp float material_diffuse_intensity;
uniform lowp float material_alpha;

in lowp vec3 v_normal;
in highp vec2 v_texcoord;
in highp vec3 v_surface_position;

out lowp vec4 frag_color;

void main() {
    lowp vec3 diffuse_light_color = vec3(0, 0, 0);
    for (int i = 0; i < num_lights; i++) {
        diffuse_light_color += lights[i].color;
    }
    diffuse_light_color *= material_diffuse_intensity;
    
    frag_color = vec4((ambient_light_color + diffuse_light_color) * material_diffuse_surface_color.xyz,
                       material_alpha * material_diffuse_surface_color.a);
}