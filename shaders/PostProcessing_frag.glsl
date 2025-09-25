# version 460 core

out vec4	ScreenColor;
in vec2		uv;

uniform vec3	sunPos;
uniform vec3	camPos;
uniform vec3	camDir;
uniform float	time;

uniform sampler2D	depthBuffer;
uniform sampler2D	postProcBuffer;
uniform sampler3D	test3D;
uniform bool		flashlightOn;
uniform mat4		view;

/// --- ANTI-ALIASING FILTERING

const float	EDGE_THRESHOLD_MIN = 0.0312;
const float	EDGE_THRESHOLD_MAX = 0.125;
const float	SUBPIXEL_QUALITY = 0.75;
const vec2	inverseScreenSize = vec2(1.0f / 1920.0f, 1.0f / 1080.0f);

float	rgbToLuma(const vec3 rgb) {
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float	QUALITY(int i) {
	i = clamp(i, 0, 8);
	float	quality[] = {0.0, 0.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0};
	return quality[i];
}

vec2	fxaaFiltering(const vec2 screenSize) {
	vec3	colorCenter = texture(postProcBuffer, uv).rgb;
	float	lumaCenter = rgbToLuma(colorCenter);	// current fragment luma

	// direct neighbour's luma
	float	lumaDown = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(0, -1)).rgb);
	float	lumaUp = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(0, 1)).rgb);
	float	lumaLeft = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(-1, 0)).rgb);
	float	lumaRight = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(1, 0)).rgb);

	// max and min luma around current fragment
	float	lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	float	lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	float	lumaRange = lumaMax - lumaMin;
	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
		return uv;

	// Query the 4 remaining corners lumas.
	float	lumaDownLeft = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(-1,-1)).rgb);
	float	lumaUpRight = rgbToLuma(textureOffset(postProcBuffer,uv, ivec2(1,1)).rgb);
	float	lumaUpLeft = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(-1,1)).rgb);
	float	lumaDownRight = rgbToLuma(textureOffset(postProcBuffer, uv, ivec2(1,-1)).rgb);

	// Combine the four edges lumas (using intermediary variables for future computations with the same values).
	float	lumaDownUp = lumaDown + lumaUp;
	float	lumaLeftRight = lumaLeft + lumaRight;

	// Same for corners
	float	lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float	lumaDownCorners = lumaDownLeft + lumaDownRight;
	float	lumaRightCorners = lumaDownRight + lumaUpRight;
	float	lumaUpCorners = lumaUpRight + lumaUpLeft;

	// Compute an estimation of the gradient along the horizontal and vertical axis.
	float	edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
	float	edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

	// Is the local edge horizontal or vertical ?
	bool	isHorizontal = (edgeHorizontal >= edgeVertical);

	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	float	luma1 = isHorizontal ? lumaDown : lumaLeft;
	float	luma2 = isHorizontal ? lumaUp : lumaRight;
	// Compute gradients in this direction.
	float	gradient1 = luma1 - lumaCenter;
	float	gradient2 = luma2 - lumaCenter;

	// Which direction is the steepest ?
	bool	is1Steepest = abs(gradient1) >= abs(gradient2);

	// Gradient in the corresponding direction, normalized.
	float	gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

	// Choose the step size (one pixel) according to the edge direction.
	float	stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	// Average luma in the correct direction.
	float	lumaLocalAverage = 0.0;

	if (is1Steepest) {
		// Switch the direction
		stepLength = - stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);

	// Shift UV in the correct direction by half a pixel.
	vec2 currentUv = uv;
	if (isHorizontal)
		currentUv.y += stepLength * 0.5;
	else
		currentUv.x += stepLength * 0.5;

	// Compute off (for each iteration step) in the right direction.
	vec2	off = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);
	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
	vec2	uv1 = currentUv - off;
	vec2	uv2 = currentUv + off;

	// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
	float	lumaEnd1 = rgbToLuma(texture(postProcBuffer, uv1).rgb);
	float	lumaEnd2 = rgbToLuma(texture(postProcBuffer, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	// If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
	bool	reached1 = abs(lumaEnd1) >= gradientScaled;
	bool	reached2 = abs(lumaEnd2) >= gradientScaled;
	bool	reachedBoth = reached1 && reached2;

	// If the side is not reached, we continue to explore in this direction.
	if (!reached1)
		uv1 -= off;
	if (!reached2)
		uv2 += off;

	// If both sides have not been reached, continue to explore.
	if (!reachedBoth) {

		for (int i = 2; i < 12; i++) {
			// If needed, read luma in 1st direction, compute delta.
			if (!reached1) {
				lumaEnd1 = rgbToLuma(texture(postProcBuffer, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			// If needed, read luma in opposite direction, compute delta.
			if (!reached2) {
				lumaEnd2 = rgbToLuma(texture(postProcBuffer, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable quality.
			if (!reached1)
				uv1 -= off * QUALITY(i);
			if (!reached2)
				uv2 += off * QUALITY(i);

			// If both sides have been reached, stop the exploration.
			if (reachedBoth)
				break ;
		}
	}

	// Compute the distances to each extremity of the edge.
	float	distance1 = isHorizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
	float	distance2 = isHorizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);

	// In which direction is the extremity of the edge closer ?
	bool	isDirection1 = distance1 < distance2;
	float	distanceFinal = min(distance1, distance2);

	// Length of the edge.
	float	edgeThickness = (distance1 + distance2);
	
	// UV off: read in the direction of the closest side of the edge.
	float	pixelOffset = - distanceFinal / edgeThickness + 0.5;

	// Is the luma at center smaller than the local average ?
	bool	isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	
	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	// (in the direction of the closer side of the edge.)
	bool	correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

	// If the luma variation is incorrect, do not off.
	float	finalOffset = correctVariation ? pixelOffset : 0.0;

	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	float	lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	float	subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	float	subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	// Compute a sub-pixel off based on this delta.
	float	subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	// Pick the biggest of the two offs.
	finalOffset = max(finalOffset, subPixelOffsetFinal);

	// Compute the final UV coordinates.
	vec2	finalUv = uv;
	if (isHorizontal)
		finalUv.y += finalOffset * stepLength;
	else
		finalUv.x += finalOffset * stepLength;
	return finalUv;
}

/// --- COSMETIC FILTERING

vec3	chromaticAberationFilter(vec2 localUV) {
	vec2	center = vec2(0.5);
	float	d = pow(distance(localUV, center) * 0.75, 2.3f);

	float	red = texture(postProcBuffer, localUV + (0.01 * d)).r;
	float	blue = texture(postProcBuffer, localUV - (0.01 * d)).b;
	
	return vec3(red, texture(postProcBuffer, localUV).g, blue);
}

const float	GAMMA = 0.6f;
const float	COLOR_NUM = 256.0f;

vec3	posterizationFilter(vec3 baseColor) {
	baseColor = pow(baseColor, vec3(GAMMA));
	baseColor *= COLOR_NUM;
	baseColor = floor(baseColor);
	baseColor /= COLOR_NUM;
	baseColor = pow(baseColor, vec3(1.0 / GAMMA));
	return	baseColor;
}

/// --- CLOUD RENDERING

// Cloud bounding box parameters
const vec3	boundingBoxPosition = vec3(0, 450, 0);
const vec3	boundingBoxSize = vec3(5000, 35, 5000);

// Cloud Noise parameters
const float	densityThreshold = 350.0f;
const float	densityMultiplier = 0.00005f;
const float	cloudScale = 0.00015f;
const float	cloudSpeed = 0.002;

// Cloud parameters
const float	phaseVal = 0.08;
const float	cloudLightAbsorbtion =  1.14;
const float	numStepRay = 512.0f;

// Light parameters
float	numStepLight = 16.0f;
float	sunLightAbsorbtion = 0.06f;
float	darknessThreshold = 12.018f;

float	sampleDensity(vec3 position) {
	vec3	uvw1 = position * cloudScale + vec3(time * cloudSpeed, 0.0, time * cloudSpeed);
	vec3	uvw2 = position * (cloudScale * 2.0f) + vec3(time * 0.02, 10.0, time * 0.02);
	vec4	shape1 = texture(test3D, uvw1);
	vec4	shape2 = texture(test3D, uvw2);
	return max(0, shape1.r - densityThreshold) * densityMultiplier;
}

vec2	rayBoxDist(vec3 boundsMin, vec3 boundsMax, vec3 ro, vec3 rd) {
	vec3	t0 = (boundsMin - ro) / rd;
	vec3	t1 = (boundsMax - ro) / rd;
	vec3	tmin = min(t0, t1);
	vec3	tmax = max(t0, t1);

	float	dstA = max(max(tmin.x, tmin.y), tmin.z);
	float	dstB = min(tmax.x, min(tmax.y, tmax.z));

	float	dstToBox = max(0, dstA);
	return vec2(max(0, dstA), max(0, dstB - dstToBox));
}

float	screenToEyeDepth(float d, float near, float far) {
	float	depth = d * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - depth * (far - near));
}

vec3	cloudRayMarching(vec2 localUV) {
	// Account for aspect ratio
	localUV.x *= 1920.0 / 1080.0;

	// Raymarch camera initialization
	vec4	sampledDepthBuffer = texture(depthBuffer, uv);
	vec3	ro = camPos;
	float	d = 1.0 / tan(1.3962634 / 2.0);
	vec4	rd = normalize(vec4(localUV, -d, 1.0f)) * view;

	// Cloud bounding box initialization
	vec3	boundsMin = boundingBoxPosition - boundingBoxSize;
	vec3	boundsMax = boundingBoxPosition + boundingBoxSize;
	vec2	rayBoxInfo = rayBoxDist(boundsMin, boundsMax, ro, rd.xyz);

	// Ray initialization
	float	travel = 0;
	float	stepSize = rayBoxInfo.y / numStepRay;
	float	depth = screenToEyeDepth(sampledDepthBuffer.r, 0.1f, 10000.0f);
	float	limit = min(depth - rayBoxInfo.x, rayBoxInfo.y);

	float	transmittance = 1.0f;
	float	lightEnergy = 0.0;
	
	while (travel < limit) {
		vec3	rayPos = ro + rd.xyz * vec3(rayBoxInfo.x + travel);
		float	density = sampleDensity(rayPos);

		if (density > 0) {
			float	lightTransmittance = (rayPos.y * sunLightAbsorbtion) - darknessThreshold;
			lightEnergy += density * stepSize * transmittance * lightTransmittance * phaseVal;
			transmittance *= exp(-density * stepSize * cloudLightAbsorbtion);

			if (transmittance < 0.01)
				break ;
		}

		travel += stepSize;
	}
	vec3	cloudColor = vec3(lightEnergy);
	return vec3(transmittance) + cloudColor;
}

/// --- SHADER MAIN FUNCTION

void	main() {
	vec2	filterdUV = fxaaFiltering(textureSize(postProcBuffer, 0));
	vec3	color = texture(postProcBuffer, filterdUV).rgb;

	color = posterizationFilter(color);
	color *= cloudRayMarching(filterdUV * 2.0 - 1.0);

	/// --- FINAL COLOR CALCULATION

	ScreenColor = vec4(color, 1.0f);
	// ScreenColor = vec4(vec3(depth), 1.0f);
}
