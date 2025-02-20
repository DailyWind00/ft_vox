# version 460 core

out float	FragColor;

in vec2		uv;

uniform sampler2D	gPosition;
uniform sampler2D	gNormal;
uniform sampler2D	Noise;

uniform vec3	samples[64];
uniform mat4	transform;

const vec2	noiseScale = vec2(1920.0f / 4.0f, 1080.0f / 4.0f);
const float	bias = 0.025;

void	main()
{
	vec3	fragPos = texture(gPosition, uv).rgb;
	vec3	normal = texture(gNormal, uv).rgb;
	vec3	randomVec = texture(Noise, uv * noiseScale).rgb;

	vec3	tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3	bitangent = cross(normal, tangent);
	mat3	TBN = mat3(tangent, bitangent, normal);

	float	occlusion = 0.0f;

	for (int i = 0; i < 64; i++) {
		vec3	samplePos = TBN * samples[i];

		samplePos = fragPos + samplePos * 0.5;

		vec4	off = vec4(samplePos, 1.0f);

		off = transform * off;
		off.xyz /= off.w;
		off.xyz = off.xyz * 0.5 + 0.5;

		float	sampleDepth = texture(gPosition, off.xy).z;

		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f);
	}
	occlusion = 1.0f - (occlusion / 64.0f);
	//-FragColor = occlusion;
	FragColor = normal.x + normal.y + normal.z;
}
