#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gEmissive;

in vec3	fragPos;
in vec3	Normal;
in vec2	uv;
in vec2	l;
flat in uint	texID;
flat in uint	face;

uniform mat4	view;
uniform float		time;
uniform vec3		sunPos;
uniform vec3		camPos;
uniform bool		polygonVisible;
uniform sampler2D	atlas;

float	sdfSegment(vec2 p, vec2 a, vec2 b) {
	float	h = min(1.0, max(0.0, dot(p - a, b - a) / dot(b - a, b - a)));
	return length(p - (a + h * (b - a)));
}

float	addSegment(float d1, float d2, float d3, float d4, float d5) {
	return min(min(min(min(d1, d2), d3), d4), d5);
}

vec3 getSkyGradient(vec3 direction, float sunHeight) {
	// Colors for day, twilight, and night sky
	vec3 horizonColorDay = vec3(0.1, 0.2, 0.5);   // Near the horizon (darker blue)
	vec3 skyColorDay = vec3(0.8, 0.6, 0.9);         // Above the horizon (blue)
	vec3 zenithColorDay = vec3(0.45, 0.75, 0.95);   // Directly above (lighter blue)
	
	// Colors for twilight (dawn and dusk)
	vec3 horizonColorTwilight = vec3(0.4, 0.2, 0.1); // Warm orange hues
	vec3 skyColorTwilight = vec3(0.7, 0.4, 0.3);     // Pinkish light
	vec3 zenithColorTwilight = vec3(0.5, 0.3, 0.2);   // Soft purple/pink

	// Darker night colors
	vec3 horizonColorNight = vec3(0.0, 0.0, 0.1);   // Darker near the horizon
	vec3 skyColorNight = vec3(0.0, 0.0, 0.2);       // Dark blue for night sky
	vec3 zenithColorNight = vec3(0.05, 0.05, 0.2);   // Darker blue near zenith

	// Transition based on sun height
	float twilightStart = -0.50; // Sun height threshold for twilight start (dusk and dawn)
	float twilightEnd = 0.50;   // Sun height threshold for twilight end (dawn and dusk)
	float twilightMid = twilightStart + twilightEnd; // Midpoint between twilight start and end

	// Smooth transitions based on sun position for each phase
	vec3 horizonColor, skyColor, zenithColor;

	if (sunHeight > twilightEnd) { // Day
		horizonColor = horizonColorDay;
		skyColor = skyColorDay;
		zenithColor = zenithColorDay;
	}
	else if (sunHeight > twilightStart) { // Twilight (Dusk or Dawn)
		if (sunHeight > twilightMid) { // Day to twilight
			float t = smoothstep(twilightEnd, twilightMid, sunHeight);
			horizonColor = mix(horizonColorDay, horizonColorTwilight, t);
			skyColor     = mix(skyColorDay, skyColorTwilight, t);
			zenithColor  = mix(zenithColorDay, zenithColorTwilight, t);
		}
		else { // Twilight to night
			float t = smoothstep(twilightStart, twilightMid, sunHeight);
			horizonColor = mix(horizonColorNight, horizonColorTwilight, t);
			skyColor     = mix(skyColorNight, skyColorTwilight, t);
			zenithColor  = mix(zenithColorNight, zenithColorTwilight, t);
		}
	}
	else { // Night
		horizonColor = horizonColorNight;
		skyColor = skyColorNight;
		zenithColor = zenithColorNight;
	}

	// Smooth transitions between skyColor and zenithColor based on direction
	float t = smoothstep(0.0, 1.0, direction.y * 0.5 + 0.5);
	t = smoothstep(0.0, 1.0, t); // Apply smoothstep twice for a softer gradient

	// Mix between skyColor and zenithColor depending on the direction
	vec3 mixedSkyColor = mix(skyColor, zenithColor, t);

	return mix(horizonColor, mixedSkyColor, t);
}

// Function to compute the sun's brightness and color
vec3 getSunColor(vec3 direction, vec3 sunPos) {
	vec3	sunColor = vec3(1.0, 0.9, 0.6);
	float	dist = length(direction - sunPos * 0.99) * 0.15;
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

float random (in vec2 st) {
	return fract(sin(dot(st.xy, vec2(12.9898,78.233)))
			* 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float	noise(in vec2 st) {
	vec2	i = floor(st);
	vec2	f = fract(st);

	// Four corners in 2D of a tile
	float	a = random(i);
	float	b = random(i + vec2(1.0, 0.0));
	float	c = random(i + vec2(0.0, 1.0));
	float	d = random(i + vec2(1.0, 1.0));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	vec2	u = f * f * (3.0 - 2.0 * f);
	// u = smoothstep(0.,1.,f);

	// Mix 4 coorners percentages
	return mix(a, b, u.x) +
		(c - a)* u.y * (1.0 - u.x) +
		(d - b) * u.x * u.y;
}

float	noise3D(const vec3 stb) {
	float a = noise(stb.xy);
	float b = noise(stb.yz);
	float c = noise(stb.xz);
	float d = noise(stb.yx);
	float e = noise(stb.zy);
	float f = noise(stb.zx);
	return (a + b + c + d + e + f) / 6.0f;
}

void	main()
{
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);
	gEmissive = vec4(0.0);

	uint	xOff = texID % 16;
	uint	yOff = texID / 16;

	vec4	viewFPos = view * vec4(fragPos, 1.0f);
	float	thickness = (0.0003 / ((l.x + l.y) / 2.0f)) * (-viewFPos.z * 4);
	float	roughness = 0.003 / ((l.x + l.y) / 2.0f) * (-viewFPos.z * 2);

	vec3	color = texture(atlas, vec2((fract(uv.x * l.x) + xOff) / 16, (fract(uv.y * l.y) + yOff) / 16)).rgb ;

	if (polygonVisible) {
		float	d = addSegment(sdfSegment(uv, vec2(0, 0), vec2(1, 0)),
			sdfSegment(uv, vec2(0, 0), vec2(0, 1)),
			sdfSegment(uv, vec2(1, 1), vec2(1, 0)),
			sdfSegment(uv, vec2(1, 1), vec2(0, 1)),
			sdfSegment(uv, vec2(0, 0), vec2(1, 1)));
		float	sm = 1.0f - smoothstep(thickness, (thickness + roughness), d);
		vec3	polygonColor = (1.0f - color) * sm;
		color *= 1.0f - sm;
		gEmissive = vec4(polygonColor, 1.0f);
	}

	float	alpha = 1.0f;
	vec4	finalColor = vec4(color, 1.0f);
	if (texID == 8) {
		float	largeNoise = noise3D(vec3(fragPos.x + time * 6.0f, fragPos.y + time * 12.0f, fragPos.z + time * 6.0f) * 0.05) * 0.5;

		vec3	waveNormal = Normal * vec3(largeNoise);
		gNormal = vec4(waveNormal, 1.0f);
		vec3	reflectedColor = getSkyGradient(vec3(0, 1, 0), sunPos.y);

		vec3	viewVector = normalize(camPos - fragPos);
		float	fresnel = clamp(pow(1.0f - dot(viewVector, waveNormal), 1.4f), 0.0f, 1.0f);
		finalColor = mix(vec4(color, 0.45), vec4(reflectedColor, 1.0f), fresnel);
	}

	gColor = finalColor;
}
