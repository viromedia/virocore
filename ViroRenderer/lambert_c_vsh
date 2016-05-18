#version 300 es

in vec3 position;
in vec3 normal;
in vec2 texcoord;

uniform mat4 normal_matrix;
uniform mat4 model_matrix;
uniform mat4 modelview_projection_matrix;

out vec3 v_normal;
out vec2 v_texcoord;
out vec3 v_surface_position;

void main() {
    gl_Position = modelview_projection_matrix * vec4(position, 1.0);
    v_texcoord = texcoord;
    v_surface_position = (model_matrix * vec4(position, 1.0)).xyz;
    v_normal = normalize((normal_matrix * vec4(normal, 0.0)).xyz);
}