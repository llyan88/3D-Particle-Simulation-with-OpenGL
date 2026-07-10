#version 330 core

// ---------------------------------------------------------------------------
// Particle vertex shader
//   Receives per-particle position, colour, and base size.
//   Computes perspective-correct point size so particles appear the same
//   physical diameter regardless of distance from the camera.
// ---------------------------------------------------------------------------

layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec4  aColor;
layout(location = 2) in float aSize;

out vec4 vColor;

uniform mat4  uView;
uniform mat4  uProjection;
uniform float uScreenHeight;   // viewport height in pixels

void main() {
    vec4 eyePos   = uView * vec4(aPosition, 1.0);
    gl_Position   = uProjection * eyePos;

    // Perspective-correct sizing: larger = closer, smaller = farther
    // We multiply by the projection's focal length (approx. screenHeight / (2*tan(fov/2)))
    // Simplified: divide base size by -eyePos.z (distance along view axis)
    float dist     = max(0.1, -eyePos.z);
    gl_PointSize   = max(1.0, aSize * (uScreenHeight * 0.5) / dist);

    vColor = aColor;
}
