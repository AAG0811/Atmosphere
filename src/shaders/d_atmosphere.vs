#version 330 core

layout(location = 0) in vec3 aPos;  // Position of the vertex

uniform mat4 model;         // Model matrix for the atmosphere sphere
uniform mat4 view;          // View matrix
uniform mat4 projection;    // Projection matrix
uniform vec3 cameraPos;     // Position of the camera in world space
uniform vec3 ulightDirection;      // Direction of the light in world space

out vec3 fragPos;           // Position of the fragment in world space
out vec3 viewDir;           // View direction from fragment to camera
out vec3 lightDirection;    // Light direction in world space

void main() {
    // Calculate the position of the vertex in world space
    fragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate view direction from the fragment to the camera
    viewDir = normalize(cameraPos - fragPos);
    
    // Light direction (assumed constant for a directional light)
    lightDirection = normalize(ulightDirection);

    // Final position in clip space
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
