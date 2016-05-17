fragment float4 lambert_lighting_fragment_t_reflect(VROLambertLightingVertexOut in [[ stage_in ]],
                                                    texture2d<float> texture [[ texture(0) ]],
                                                    texturecube<float> reflect_texture [[ texture(1) ]],
                                                    constant VROSceneLightingUniforms &lighting [[ buffer(0) ]]) {
    
    float4 reflective_color = compute_reflection(in.surface_position, in.camera_position, in.normal, reflect_texture);
    float4 lighting_color = lambert_lighting_diffuse_texture(in, texture, lighting);
    
    return float4(lighting_color.xyz + reflective_color.xyz, lighting_color.a);
}