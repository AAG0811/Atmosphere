#version 330 core

in vec3 fragPos;            // Fragment position in world space
in vec3 viewDir;            // Normalized view direction from fragment to camera
in vec3 lightDirection;     // Normalized light direction vector from sun to atmosphere

uniform float planetRadius;           // Radius of the planet
uniform float atmosphereRadius;       // Radius of the atmosphere
uniform vec3 rayleighCoefficient;     // Rayleigh scattering coefficient (per channel)
uniform vec3 mieCoefficient;          // Mie scattering coefficient (per channel)
uniform vec3 rayleighColorFactor;     // Color tint for Rayleigh scattering
uniform vec3 mieColorFactor;          // Color tint for Mie scattering
uniform float mieDirectionality;      // Directionality of Mie scattering
uniform int numSamples;               // Number of view ray samples
uniform int numLightSamples;          // Number of light ray samples

out vec4 FragColor;

const float PI = 3.14159;
const float SCALE_HEIGHT = 1.0;       // Scale height for Rayleigh scattering falloff

// Density function for the atmosphere based on altitude
float density(float height) {
    return exp(-height / SCALE_HEIGHT);
}

// Rayleigh scattering phase function
float rayleighPhase(float cosTheta) {
    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
}

// Mie scattering phase function
float miePhase(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Computes the intersection distance to the atmosphere boundary along the view ray
float computeViewRayLength(vec3 origin, vec3 dir, float radius) {
    float a = dot(dir, dir);
    float b = 2.0 * dot(origin, dir);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;
    
    // Check if ray intersects the sphere
    if (discriminant < 0.0) {
        return 0.0; // No intersection
    }
    
    // Calculate intersection points (we want the closest positive distance)
    float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0 * a);
    
    // Return the nearest positive intersection distance
    if (t1 > 0.0) return t1;
    if (t2 > 0.0) return t2;
    return 0.0; // Both intersections are behind the origin
}

void main() {
    float fragHeight = length(fragPos) - planetRadius;  // Altitude of the fragment
    bool insideAtmosphere = length(fragPos) < atmosphereRadius;
    
    float viewRayLength;
    if (insideAtmosphere) {
        // Calculate distance to atmosphere's outer boundary along the view direction
        viewRayLength = computeViewRayLength(fragPos, viewDir, atmosphereRadius);
    } else {
        // Default to a fixed view length (e.g., large distance to space) for simplicity
        viewRayLength = 1000.0;  // Large arbitrary value if outside the atmosphere
    }

    vec3 accumulatedRayleigh = vec3(0.0);
    vec3 accumulatedMie = vec3(0.0);
    float opticalDepthRayleigh = 0.0;
    float opticalDepthMie = 0.0;
    
    // Calculate step size for sampling along the view ray
    float stepSize = viewRayLength / float(numSamples);
    vec3 samplePoint = fragPos;
    
    for (int i = 0; i < numSamples; i++) {
        // Sample point along the view ray
        samplePoint = fragPos + viewDir * stepSize * float(i);
        float height = length(samplePoint) - planetRadius;
        
        // Calculate density based on height
        float densitySample = density(height);
        opticalDepthRayleigh += densitySample * stepSize;
        opticalDepthMie += densitySample * stepSize;
        
        // Integrate scattering due to sunlight at this point
        float lightRayleighDepth = 0.0;
        float lightMieDepth = 0.0;
        vec3 lightSample = samplePoint;
        float lightStepSize = stepSize / float(numLightSamples);
        
        for (int j = 0; j < numLightSamples; j++) {
            lightSample += lightDirection * lightStepSize;
            float lightHeight = length(lightSample) - planetRadius;
            float densityAlongLight = density(lightHeight);
            lightRayleighDepth += densityAlongLight * lightStepSize;
            lightMieDepth += densityAlongLight * lightStepSize;
        }
        
        // Phase functions for scattering
        // float cosTheta = dot(viewDir, lightDirection);
        float cosTheta = 1.0;
        float rayleighPhaseFactor = rayleighPhase(cosTheta);
        float miePhaseFactor = miePhase(cosTheta, mieDirectionality);
        
        // Calculate Rayleigh and Mie scattering contributions
        vec3 rayleighScattering = rayleighCoefficient * rayleighColorFactor * rayleighPhaseFactor * densitySample * exp(-lightRayleighDepth * rayleighCoefficient);
        vec3 mieScattering = mieCoefficient * mieColorFactor * miePhaseFactor * densitySample * exp(-lightMieDepth * mieCoefficient);
        
        accumulatedRayleigh += rayleighScattering * stepSize;
        accumulatedMie += mieScattering * stepSize;
    }
    
    // Combine Rayleigh and Mie contributions
    vec3 color = accumulatedRayleigh + accumulatedMie;
    
    // Gamma correction for better color accuracy
    // FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
    vec3 simpleMie = mieCoefficient * mieColorFactor * miePhase(dot(viewDir, lightDirection), 0.8);
    vec3 simpleRayleigh = rayleighCoefficient * rayleighColorFactor * rayleighPhase(dot(viewDir, lightDirection));
    FragColor = vec4(color, 1);
    // FragColor = vec4(vec3(opticalDepthRayleigh), 1);
}
