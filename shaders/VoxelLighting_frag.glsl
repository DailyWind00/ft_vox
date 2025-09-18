# version 460 core

out vec4	ScreenColor;
in vec2		uv;
in vec3		spFragPos;
in vec3		lpFragPos;

uniform sampler2D	gPosition;
uniform sampler2D	gNormal;
uniform sampler2D	gColor;
uniform sampler2D	shadowMap;
uniform mat4		spView;
uniform mat4		spProj;
uniform mat4		lpMat;

uniform vec3	sunPos;
uniform bool	polygonVisible;

uniform vec2	screenSize;
const float		crossThickness = 1.0f;
const float 	crossLength = 10.0f;

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

vec3	computeLighting(const vec3 texCol, const vec3 Normal) {
	float	clampedSunHeight = clamp(sunPos.y, 0.15, 0.85);

	vec3	skyCol = getSkyGradient(vec3(1.0), sunPos.y);
	vec3	Color = pow(clampedSunHeight, 0.7) * texCol + (1.0f - pow(clampedSunHeight, 0.7)) * skyCol;
	vec3	ambColor = Color.rgb * 0.4;

	float	diffuseSun = max(dot(Normal.rgb, sunPos), 0.0) * pow(sunPos.y, 1.2);
	float	diffuseMoon = max(dot(Normal.rgb, -sunPos), 0.0) * pow(-sunPos.y, 3);

	vec3	diffColor = max(diffuseSun, diffuseMoon) * Color.rgb;

	return ambColor + diffColor;
}

float	computeFogFactor(const float scDepth) {
	float	lerpFactor = scDepth * 0.001;

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

void	main()
{
	vec4	fragPos = spView * texture(gPosition, uv);
	vec4	Normal = texture(gNormal, uv);
	vec3	texCol = texture(gColor, uv).rgb;

	vec3	lightColor = computeLighting(texCol, Normal.rgb);

	float	fogFactor = computeFogFactor(-fragPos.z);

	vec4	crosshair = vec4(1.0f - computeCrosshair(), 0.75);

	ScreenColor = max(vec4(mix(lightColor, getSkyGradient(vec3(0, 0, 0), sunPos.y), fogFactor), 1.0f), crosshair);
}
