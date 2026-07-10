#version 330 core

// ---------------------------------------------------------------------------
// Particle fragment shader
//   Renders each point sprite as a soft circular disc.
//   Uses gl_PointCoord to compute distance from sprite centre, discards
//   fragments outside the circle, and applies a smooth radial falloff that
//   produces a glowing appearance when combined with additive blending.
// ---------------------------------------------------------------------------

in  vec4 vColor;
out vec4 FragColor;

void main() {
    // gl_PointCoord is in [0,1]; remap to [-1,1]
    vec2  coord = gl_PointCoord * 2.0 - 1.0;
    float r2    = dot(coord, coord);   // squared radius

    // Discard outside the unit circle
    if (r2 > 1.0) discard;

    // Soft glow: bright core fading smoothly to transparent at the edge
    float core   = 1.0 - smoothstep(0.0, 0.4, r2);   // bright inner core
    float halo   = 1.0 - smoothstep(0.4, 1.0, r2);   // wider soft halo
    float alpha  = vColor.a * (core * 0.9 + halo * 0.5);

    // Brighten the centre slightly for a "hot" look
    vec3  color  = vColor.rgb + core * 0.3;

    FragColor = vec4(color, alpha);
}
