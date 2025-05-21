#version 410 core
#include "../common/lights.glsl"

in VertexOut {
    LightingResult lighting_result;
    vec2 texture_coordinate;
} frag_in;

layout(location = 0) out vec4 out_colour;

// Global Data
uniform float inverse_gamma;

// Material properties
uniform vec2 diffuse_texture_scale;
uniform vec2 specular_texture_scale;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_map_texture;

void main() {
    // Apply texture scaling to coordinates
    vec2 scaled_diffuse_coords = frag_in.texture_coordinate * diffuse_texture_scale;
    vec2 scaled_specular_coords = frag_in.texture_coordinate * specular_texture_scale;
    
    // Use the scaled texture coordinates for sampling
    vec3 texture_colour = texture(diffuse_texture, scaled_diffuse_coords).rgb;
    vec3 specular_map_sample = texture(specular_map_texture, scaled_specular_coords).rgb;

    vec3 textured_diffuse = frag_in.lighting_result.total_diffuse * texture_colour;
    vec3 sampled_specular = frag_in.lighting_result.total_specular * specular_map_sample;
    vec3 textured_ambient = frag_in.lighting_result.total_ambient * texture_colour;

    // Mix the diffuse and ambient so that there is no ambient in bright scenes
    vec3 resolved_lighting = max(textured_diffuse, textured_ambient) + sampled_specular;

    out_colour = vec4(resolved_lighting, 1.0f);
    out_colour.rgb = pow(out_colour.rgb, vec3(inverse_gamma));
}