#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;

in vec4 worldPixel;

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform sampler2D rock, snow, grass;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;


vec3 lerp(vec3 a, vec3 b, float t)
{
	return a + (b - a) * t;
}

vec4 lerp(vec4 a, vec4 b, float t)
{
	return a + (b - a) * t;
}

vec3 triplanar(vec4 worldPixel, sampler2D tex, float scale) 
{
	vec3 zy = texture(tex, vec2(worldPixel.z, worldPixel.y) * scale).rgb;
	vec3 xz = texture(tex, vec2(worldPixel.x, worldPixel.z) * scale).rgb;
	vec3 xy = texture(tex, vec2(worldPixel.x, worldPixel.y) * scale).rgb;

	return (zy + xz + xy) / 3;
}

void main() 
{
	vec3 normalCol = texture(normalmap, uv).rbg * 2 - 1;

	normalCol.b = -normalCol.b;
	normalCol.r = -normalCol.r;

	vec3 lightDir = normalize(lightDirection);
	float light = max(dot(-lightDir, normalCol), 0.25f);

	float steepness = dot(normalCol, vec3(0, 1, 0));
	float rockCover = smoothstep(0.55, 0.75, steepness);

	float snowCover = smoothstep(0.6, 0.63, worldPixel.y / 100);
	
	vec3 grassCol = triplanar(worldPixel, grass, 0.2);
	vec3 rockCol = triplanar(worldPixel, rock, 0.2);
	vec3 snowCol = triplanar(worldPixel, snow, 0.2);

	vec3 diffuse = lerp(grassCol, snowCol, snowCover) * rockCover + rockCol * (1 - rockCover);

	float d = distance(worldPixel.xyz, cameraPosition);
	float fogAmount = clamp((d - 150) / 200, 0, 1);
	vec3 bot = vec3(146, 220, 186) / 255.0;

	FragColor = vec4(lerp(diffuse * light, bot, fogAmount), 1);
}