#version 330 core

// ---------------------------------------------------------------------------
// Bounding-box wireframe fragment shader
//   Colour is passed as a uniform so it can be changed at runtime.
// ---------------------------------------------------------------------------

out vec4 FragColor;

uniform vec4 uColor;   // RGBA line colour

void main() {
    FragColor = uColor;
}
