#version 300 es
#include constant_functions_fsh

uniform lowp float material_diffuse_intensity;
uniform lowp float material_alpha;

uniform sampler2D sampler;

in lowp vec3 v_normal;
in highp vec2 v_texcoord;
in highp vec3 v_surface_position;

out lowp vec4 frag_color;

void main() {
    lowp vec4 diffuse_texture_color = texture(sampler, v_texcoord);
    lowp vec3 material_diffuse_color = diffuse_texture_color.xyz * material_diffuse_intensity;

    lowp vec3 diffuse_light_color = vec3(0, 0, 0);
    for (int i = 0; i < num_lights; i++) {
        diffuse_light_color += lights[i].color;
    }
    diffuse_light_color *= material_diffuse_intensity;
    
    frag_color = vec4((ambient_light_color + diffuse_light_color) * material_diffuse_color.xyz,
                       material_alpha * diffuse_texture_color.a);
}