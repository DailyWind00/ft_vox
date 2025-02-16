# version 460 core

out vec4	ScreenColor;
in vec2		uv;

uniform sampler2D	gPosition;
uniform sampler2D	gNormal;
uniform sampler2D	gColor;

uniform vec3	sunPos;

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

void	main()
{
	vec4	fragPos = texture(gPosition, uv);
	vec4	Normal = texture(gNormal, uv);
	vec3	Color = texture(gColor, uv).rgb * getSkyGradient(vec3(1.0f), sunPos.y);
    	vec3	direction = normalize(fragPos.rgb);

	vec3	ambColor = Color.rgb * 0.3;

	float	diffuseSun = max(dot(Normal.rgb, sunPos), 0.0) * pow(sunPos.y, 1.5);
	float	diffuseMoon = max(dot(Normal.rgb, -sunPos), 0.0) * pow(-sunPos.y, 3);

	vec3	diffColor = (diffuseMoon + diffuseSun) * Color.rgb;

	//-ScreenColor= vec4((face * 1.8) * (0.2 * vec3(randFactor)) + ivec3(fragPos) * 0.01, 1.0);
	//-ScreenColor = vec4(vec3(0.3 * (face + 1) * (fragPos.y + 100) * 0.005), 1.0f);
	//-ScreenColor = vec4(Normal.rgb, 1.0f);
	ScreenColor = vec4(diffColor + ambColor, 1.0f);
}
