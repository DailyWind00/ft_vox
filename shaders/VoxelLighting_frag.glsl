# version 420 core

// Flowing data
out vec4	ScreenColor;
in vec2		uv;

// Uniforms
uniform sampler2D	gPosition;
uniform sampler2D	gNormal;
uniform sampler2D	gColor;
uniform sampler2D	gEmissive;
uniform sampler2D	shadowMap;

uniform mat4		spView;
uniform mat4		spProj;
uniform mat4		lpMat;

uniform float	renderDistance;
uniform vec3	camPos;
uniform vec3	sunPos;
uniform vec2	screenSize;
uniform bool	polygonVisible;
uniform bool	inWater;
uniform bool	flashlightOn;

// Constant values
const float	crossThickness = 1.0f;
const float 	crossLength = 10.0f;

const float	constant = 1.0f;
const float	linear = 0.09f;
const float	quadratic = 0.032f;

/// Functions

// Function to compute the sun's brightness and color
vec3 getSunColor(vec3 direction, vec3 sunPos) {
	vec3	sunColor = vec3(1.0, 0.9, 0.6);
	float	dist = length(direction - sunPos * 0.99) * 0.15;
	float	glow = clamp(0.01 / dist, 0.0, 1.0);

	return sunColor * glow;
}

// Function to calculate the base sky color gradient (including dawn and twilight)
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

float	computeShadows(const vec4 lpFragPos, const vec3 normal) {
	vec3	projCoords = lpFragPos.xyz / lpFragPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	// float	closestDepth = texture(shadowMap, projCoords.xy).r;
	float	currentDepth = projCoords.z;
	if (currentDepth > 1.0f)
		return 0.0f;

	float	diffFactor = dot(normal, sunPos);
	float	bias = mix(0.005, 0.0, diffFactor);
	vec2	texelSize = 1.0 / textureSize(shadowMap, 0);
	float	shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float	pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return shadow;
}

vec3	computeLighting(const vec3 texCol, const vec3 Normal, const float shadow, const vec3 fragPos) {
	float	clampedSunHeight = clamp(sunPos.y, 0.15, 0.85);

	// Point light calculation
	float	dist = length(camPos - fragPos);
	float	attenuation = (flashlightOn) ? 1.0 / (constant + linear * dist + quadratic * (dist * dist)) : 0.0f;

	vec3	colorModifier = vec3(0.8f, 0.8f, 0.8f);
	if (inWater)
		colorModifier = vec3(0.35, 0.55, 0.75f);
	
	vec3	skyCol = getSkyGradient(vec3(1.0), sunPos.y);
	vec3	Color = pow(clampedSunHeight, 0.7) * texCol + (1.0f - pow(clampedSunHeight, 0.7)) * skyCol * colorModifier;
	vec3	ambColor = Color.rgb * max(0.4, attenuation);

	float	diffuseSun = max(dot(Normal.rgb, sunPos), 0.0) * pow(sunPos.y, 1.2);
	float	diffuseMoon = max(dot(Normal.rgb, -sunPos), 0.0) * pow(-sunPos.y, 3);


	vec3	diffColor = max(diffuseSun, diffuseMoon) * Color.rgb;
	vec3	shadowCol = vec3(1.0 - shadow + 0.2);


	return ambColor + shadowCol * diffColor;
}

float	computeFogFactor(const float scDepth) {
	float	dist = 1.0f / (renderDistance * 48.0f);
	float	lerpFactor = scDepth * dist;

	lerpFactor = pow(lerpFactor, 1.2f);
	lerpFactor = clamp(lerpFactor, 0.0, 1.0f);
	return lerpFactor;
}

vec3	computeCrosshair() {
	// Crosshair
	vec2 pixelCoord = uv * screenSize;
	vec2 center = screenSize * 0.5;
	vec2 deltaFromCenter = abs(pixelCoord - center);
	float	d1 = max(step(crossThickness, deltaFromCenter.x), step(crossLength, deltaFromCenter.y));
	float	d2 = max(step(crossThickness, deltaFromCenter.y), step(crossLength, deltaFromCenter.x));
	return vec3(min(d1, d2));
}

void	main() {
	vec4	fragPos = texture(gPosition, uv);
	vec4	spFragPos = spView * fragPos;
	vec4	lpFragPos = lpMat * fragPos;
	vec4	Normal = texture(gNormal, uv);
	vec3	texCol = texture(gColor, uv).rgb;
	vec3	emissiveColor = texture(gEmissive, uv).rgb;

	float	shadow = computeShadows(lpFragPos, Normal.xyz);
	vec3	lightColor = computeLighting(texCol, Normal.rgb, shadow, fragPos.rgb);

	vec4	crosshair = vec4(1.0f - computeCrosshair(), 0.75);

	// Distance fog
	float	fogFactor = (inWater) ? clamp(computeFogFactor(-spFragPos.z) * 8.0f, 0.0f, 0.9f) : computeFogFactor(-spFragPos.z);

	// Depth fog
	float	gradientSteepness = (inWater) ? 64.0f : 1024.0f;
	float	gradientStrength = (inWater) ? 5.0f : 150.0f;
	float	depth = max(log((-fragPos.y / gradientSteepness) + 1.0f) * gradientStrength, 0.0f);
	float	waterFogFactor = (inWater) ? -computeFogFactor(depth) : clamp(computeFogFactor(depth), 0.0f, 0.60);

	vec3	depthColor = (inWater) ? vec3(64.0f, 32.0f, 16.0f) : vec3(45.0f, 25.0f, 20.0f) / 255.0f;
	vec3	fogColor = (inWater) ? vec3(16.0f / 255.0f, 32.0f / 255.0f, 64.0f / 255.0f) : getSkyGradient(vec3(0, 0, 0), sunPos.y);

	ScreenColor = max(vec4(mix(mix(lightColor, fogColor, fogFactor), depthColor, waterFogFactor) + emissiveColor, 1.0f), crosshair);
}
