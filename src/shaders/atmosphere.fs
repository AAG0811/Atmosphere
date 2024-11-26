#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 cameraPos;
uniform vec3 lightDir;             // Light direction (e.g., sun direction)
uniform float planetRadius;         // Radius of the planet
uniform float atmosphereRadius;     // Radius of the atmosphere
uniform vec3 rayleighCoefficient;   // Rayleigh scattering coefficient
uniform vec3 mieCoefficient;        // Mie scattering coefficient
uniform float mieDirectionality;    // Mie g parameter (values between 0.8 and 0.99)

const float PI = 3.14159265359;
const float rayleighPhaseConstant = 3.0 / (16.0 * PI); // For Rayleigh phase function
const float miePhaseConstant = 1.0 / (4.0 * PI);        // For Mie phase function

// Rayleigh phase function
float rayleighPhase(float cosTheta) {
    return rayleighPhaseConstant * (1.0 + cosTheta * cosTheta);
}

// Mie phase function with directionality
float miePhase(float cosTheta, float g) {
    return miePhaseConstant * ((1.0 - g * g) / pow(1.0 + g * g - 2.0 * g * cosTheta, 1.5));
}

void main() {
    // Calculate view direction from fragment to camera
    vec3 viewDir = normalize(cameraPos - FragPos);

    // Calculate distance from fragment to planet center
    float fragDistance = length(FragPos);
    float altitude = fragDistance - planetRadius;

    // Rayleigh and Mie phase calculations based on the angle between view direction and light direction
    float cosTheta = dot(viewDir, lightDir);
    float rayleighPhaseValue = rayleighPhase(cosTheta);
    float miePhaseValue = miePhase(cosTheta, mieDirectionality);

    // Calculate scattering contributions
    vec3 rayleighScatteredLight = rayleighCoefficient * rayleighPhaseValue;
    vec3 mieScatteredLight = mieCoefficient * miePhaseValue;

    // Combine Rayleigh and Mie scattering
    vec3 scatteredLight = rayleighScatteredLight + mieScatteredLight;

    // Apply an alpha based on the atmosphere's thickness to simulate fading with altitude
    float atmosphereThickness = atmosphereRadius - planetRadius;
    float alpha = clamp(altitude / atmosphereThickness, 0.0, 1.0);

    // Set final color with transparency
    FragColor = vec4(scatteredLight, alpha * 0.5); // Use 0.5 for alpha blending; adjust as needed
}