#version 330 core

layout(location = 0) in vec3 aPos;  // Position of the vertex

uniform mat4 model;         // Model matrix for the atmosphere sphere
uniform mat4 view;          // View matrix
uniform mat4 projection;    // Projection matrix

out vec3 fragPos;           // Position of the fragment in world space

void main() {
    // Calculate the position of the vertex in world space
    fragPos = vec3(model * vec4(aPos, 1.0));
    // Final position in clip space
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
