#version 460 core

in vec3 fPos;
out vec4 FragColor;

uniform float time;

// Constants
const float PI = 3.14159265;
const float dayDuration = 10.0; // Duration of a full day cycle in seconds

// Function to generate pseudo-random noise based on fragment position
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Function to get the sun position based on time
vec3 getSunPosition(float time) {
    float angle = (time / dayDuration) * PI; // Rotate from -90° to 90°
    return normalize(vec3(cos(angle), sin(angle), 0.0));
}

// Function to compute the sun's brightness and color
vec3 getSunColor(vec3 direction, vec3 sunPos) {
    float sunIntensity = max(dot(direction, sunPos), 0.0);
    return vec3(1.0, 0.9, 0.6) * pow(sunIntensity, 128.0);
}

// Function to calculate the base sky color gradient
vec3 getSkyGradient(vec3 direction) {
    vec3 horizonColor = vec3(0.1, 0.2, 0.5);  // Near the horizon (darker blue)
    vec3 zenithColor = vec3(0.2, 0.5, 0.9);   // Directly above (lighter blue)
    
    // Use smoother interpolation with double smoothstep for extra softness
    float t = smoothstep(0.0, 1.0, direction.y * 0.5 + 0.5);
    t = smoothstep(0.0, 1.0, t); // Apply smoothstep twice for a softer gradient
    
    return mix(horizonColor, zenithColor, t);
}

void main() {
    vec3 direction = normalize(fPos);
    vec3 sunPos = getSunPosition(time);

    vec3 skyColor = getSkyGradient(direction);
    vec3 sunColor = getSunColor(direction, sunPos);

    // Add dithering to reduce color banding
    float noise = random(gl_FragCoord.xy) * (1.0 / 255.0); // Subtle noise
    skyColor += noise;

    FragColor = vec4(skyColor + sunColor, 1.0);
}