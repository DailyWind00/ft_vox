#version 460 core

in vec3 TexCoords;
out vec4 FragColor;

uniform float time;
uniform samplerCube cubemap;

// Random function for dithering
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 direction = normalize(TexCoords);
    float t = direction.y * 0.5 + 0.5;

    // Enhanced colors
    vec3 dayColor = vec3(0.1, 0.5, 1.0);       // Deeper blue sky
    vec3 sunsetColor = vec3(0.8, 0.4, 0.6);    // Softer sunset
    vec3 nightColor = vec3(0.01, 0.01, 0.05);  // Darker night

    // Controlled brightness (darker nights)
    float brightness = clamp(0.3 + 0.7 * sin(time * 0.1), 0.1, 1.0);

    // Adjusted blending for bluer sky
    vec3 skyColor = mix(nightColor, mix(dayColor, sunsetColor, smoothstep(0.0, 0.15, t)), brightness);

    // Sun movement across the horizon (Y-axis rotation)
    vec3 sunDirection = normalize(vec3(cos(time * 0.1), sin(time * 0.1), 0.0));
    float sunIntensity = max(dot(direction, sunDirection), 0.0);
    vec3 sunGlow = vec3(1.0, 0.95, 0.85) * pow(sunIntensity, 90.0);  // Sharper, tighter glow

    // Dithering to reduce color banding
    float noise = rand(TexCoords.xy * time) * 0.003;

    // Final color output with gamma correction
    vec3 finalColor = skyColor + sunGlow + noise;
    FragColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1.0);
}
