#version 410 core

in VertexOut {
    vec3 ws_position;
    vec2 texture_coordinate;
} frag_in;

layout(location = 0) out vec4 out_colour;

// Material properties
uniform vec3 emissive_tint;
uniform vec2 emission_texture_scale;

// Global Data
uniform float inverse_gamma;

uniform sampler2D emissive_texture;

void main() {
    // Apply texture scaling to coordinates
    vec2 scaled_texture_coords = frag_in.texture_coordinate * emission_texture_scale;
    
    vec3 texture_colour = texture(emissive_texture, scaled_texture_coords).rgb;
    vec3 emissive_colour = emissive_tint * texture_colour;

    out_colour = vec4(emissive_colour, 1.0f);
    out_colour.rgb = pow(out_colour.rgb, vec3(inverse_gamma));
}