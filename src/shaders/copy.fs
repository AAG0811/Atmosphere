#version 330 core

#define M_PI 3.1415926535897932384626433832795

in vec3 fragPos;     // Position of the fragment
//in vec3 fsNormal;
//in vec2 fsTexCoord;

out vec4 FragColor;

// TODO other constants
uniform vec3 viewPos;   // Position of the viewer
uniform vec3 sunPos;    // Position of the sun, light direction

// Number of samples along the view ray and light ray
uniform int viewSamples;
uniform int lightSamples;

uniform float sunIntensity;    // Intensity of the sun
uniform float planetRadius;      // Radius of the planet [m]
uniform float atmosphereRadius;      // Radius of the atmosphere [m]
uniform vec3  rCoeff;   // Rayleigh scattering coefficient
uniform float mCoeff;   // Mie scattering coefficient
uniform float rHeight;      // Rayleigh scale height
uniform float mHeight;      // Mie scale height
uniform float g;        // Mie scattering direction - 
                        //  - anisotropy of the medium

uniform float toneMappingFactor;    ///< Whether tone mapping is applied

/**
 * @brief Computes intersection between a ray and a sphere
 * @param o Origin of the ray
 * @param d Direction of the ray
 * @param r Radius of the sphere
 * @return Roots depending on the intersection
 */
vec2 raySphereIntersection(vec3 o, vec3 d, float r)
{
    // Solving analytically as a quadratic function
    //  assumes that the sphere is centered at the origin
    // f(x) = a(x^2) + bx + c
    float a = dot(d, d);
    float b = 2.0 * dot(d, o);
    float c = dot(o, o) - r * r;

    // Discriminant or delta
    float delta = b * b - 4.0 * a * c;

    // Roots not found
    if (delta < 0.0) {
      // TODO
      return vec2(1e5, -1e5);
    }

    float sqrtDelta = sqrt(delta);
    // TODO order??
    return vec2((-b - sqrtDelta) / (2.0 * a),
                (-b + sqrtDelta) / (2.0 * a));
}

/**
 * @brief Function to compute color of a certain view ray
 * @param ray Direction of the view ray
 * @param origin Origin of the view ray
 * @return color of the view ray
 */
vec3 computeSkyColor(vec3 ray, vec3 origin)
{
    // Normalize the light direction
    vec3 sunDir = normalize(sunPos);

    vec2 t = raySphereIntersection(origin, ray, atmosphereRadius);
    // Intersects behind
    if (t.x > t.y) {
        return vec3(0.0, 0.0, 0.0);
    }

    // Distance between samples - length of each segment
    t.y = min(t.y, raySphereIntersection(origin, ray, planetRadius).x);
    float segmentLen = (t.y - t.x) / float(viewSamples);

    // TODO t min
    float tCurrent = 0.0f; 

    // Rayleigh and Mie contribution
    vec3 sum_R = vec3(0);
    vec3 sum_M = vec3(0);

    // Optical depth 
    float optDeptrHeight = 0.0;
    float optDeptmHeight = 0.0;

    // Mu: the cosine angle between the sun and ray direction
    float mu = dot(ray, sunDir);
    float mu_2 = mu * mu;
    
    //--------------------------------
    // Rayleigh and Mie Phase functions
    float phase_R = 3.0 / (16.0 * M_PI) * (1.0 + mu_2);

    float g_2 = g * g;
    float phase_M = 3.0 / (8.0 * M_PI) * 
                          ((1.0 - g_2) * (1.0 + mu_2)) / 
                          ((2.0 + g_2) * pow(1.0 + g_2 - 2.0 * g * mu, 1.5));
    // Sample along the view ray
    for (int i = 0; i < viewSamples; ++i)
    {
        // Middle point of the sample position
        vec3 vSample = origin + ray * (tCurrent + segmentLen * 0.5);

        // Height of the sample above the planet
        float height = length(vSample) - planetRadius;

        // Optical depth for Rayleigh and Mie scattering for current sample
        float rHeight = exp(-height / rHeight) * segmentLen;
        float mHeight = exp(-height / mHeight) * segmentLen;
        optDeptrHeight += rHeight;
        optDeptmHeight += mHeight;

        //--------------------------------
        // Secondary - light ray
        float segmentLenLight = 
            raySphereIntersection(vSample, sunDir, atmosphereRadius).y / float(lightSamples);
        float tCurrentLight = 0.0;

        // Light optical depth 
        float optDepthLight_R = 0.0;
        float optDepthLight_M = 0.0;

        // Sample along the light ray
        for (int j = 0; j < lightSamples; ++j)
        {
            // Position of the light ray sample
            vec3 lSample = vSample + sunDir * 
                           (tCurrentLight + segmentLenLight * 0.5);
            // Height of the light ray sample
            float heightLight = length(lSample) - planetRadius;

            // TODO check sample above the ground
            
            optDepthLight_R += exp(-heightLight / rHeight) * segmentLenLight;
            optDepthLight_M += exp(-heightLight / mHeight) * segmentLenLight;

            // Next light sample
            tCurrentLight += segmentLenLight;
        }
        // TODO check sample above ground

        // Attenuation of the light for both Rayleigh and Mie optical depth
        //  Mie extenction coeff. = 1.1 of the Mie scattering coeff.
        vec3 att = exp(-(rCoeff * (optDeptrHeight + optDepthLight_R) + 
                         mCoeff * 1.1f * (optDeptmHeight + optDepthLight_M)));
        // Accumulate the scattering 
        sum_R += rHeight * att;
        sum_M += mHeight * att;

        // Next view sample
        tCurrent += segmentLen;
    }

    return sunIntensity * (sum_R * rCoeff * phase_R + sum_M * mCoeff * phase_M);
}

void main()
{
    vec3 acolor = computeSkyColor(normalize(fragPos - viewPos), viewPos);

    // Apply tone mapping
    acolor = mix(acolor, (1.0 - exp(-1.0 * acolor)), toneMappingFactor);

    FragColor = vec4(acolor, 1.0);
}