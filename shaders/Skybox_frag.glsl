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

// Function to compute the sun's brightness and color
vec3 getSunColor(vec3 direction, vec3 sunPos) {
	vec3	sunColor = vec3(1.0, 0.9, 0.6);
	float	dist = length(direction - sunPos) * 0.15;
	float	glow = clamp(0.01 / dist, 0.0, 1.0);

	return sunColor * glow;
}

// Function to compute the moon's brightness and color (solid, constant)
vec3 getMoonColor(vec3 direction, vec3 moonPos) {
	vec3	moonColor = vec3(1.0, 0.9, 0.6);
	float	dist = length(direction - moonPos) * 0.5;
	float	glow = clamp(0.01 / dist, 0.0, 1.0);

	return moonColor * glow;
}

// Function to calculate the base sky color gradient
vec3 getSkyGradient(vec3 direction) {
	vec3 horizonColor = vec3(0.1, 0.2, 0.5);   // Near the horizon (darker blue)
	vec3 skyColor = vec3(0.8, 0.6, 0.9);       // Above the horizon (blue)
	vec3 zenithColor = vec3(0.45, 0.75, 0.95); // Directly above (lighter blue)

	// Use smoother interpolation with double smoothstep for extra softness
	float t = smoothstep(0.0, 1.0, direction.y * 0.5 + 0.5);
	t = smoothstep(0.0, 1.0, t); // Apply smoothstep twice for a softer gradient

	// Mix between skyColor and zenithColor depending on the direction
	vec3 mixedSkyColor = mix(skyColor, zenithColor, t);

	return mix(horizonColor, mixedSkyColor, t);
}

void main() {
	// Variables
    float	angle = (time / dayDuration) * PI;
    vec3	sunPos = normalize(vec3(cos(angle), sin(angle), 0.0f));
    vec3	moonPos = -sunPos;
    vec3	direction = normalize(fPos);

	// Colors
    vec3	skyColor = getSkyGradient(direction);
    vec3	sunColor = getSunColor(direction, sunPos);
	vec3	moonColor = getMoonColor(direction, moonPos);

    // Add dithering to reduce color banding
    float noise = (random(gl_FragCoord.xy)) * (1.0 / 255.0); // Center noise around zero
    skyColor += noise;

    FragColor = vec4(skyColor + sunColor + moonColor, 1.0);
}