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

void main() 
{
	//vec4 diffuse = texture(heightmap, uv);
	vec3 normalCol = texture(normalmap, uv).rbg * 2 - 1;

	normalCol.b = -normalCol.b;
	normalCol.r = -normalCol.r;

	vec3 lightDir = normalize(lightDirection);
	float light = max(dot(-lightDir, normalCol), 0.25f);

	float steepness = dot(normalCol, vec3(0, 1, 0));
	float rockCover = smoothstep(0.55, 0.75, steepness);

	float scale = 0.1;

	vec2 xyUV = vec2(worldPixel.x, worldPixel.y) * scale;
	vec2 xzUV = vec2(worldPixel.x, worldPixel.z) * scale;
	vec2 yzUV = vec2(worldPixel.y, worldPixel.z) * scale;
	
	//vec3 rockCol = texture(rock, uv).rgb;
	//vec3 snowCol = texture(snow, uv).rgb;
	vec3 grassCol = texture(grass, uv).rgb;
	vec3 rockXY = texture(rock, xyUV).rgb;
	vec3 rockXZ = texture(rock, xzUV).rgb;
	vec3 rockYZ = texture(rock, yzUV).rgb;

	vec3 rockCol = (rockXY * rockXZ) * rockYZ;

	//float gr = clamp((worldPixel.y - 12) / 10, 0, 1);
	//float rs = clamp((worldPixel.y - 50) / 10, 0, 1);

	//vec3 diffuse = lerp(lerp(grassCol, rockCol, gr), snowCol, rs);
	vec3 diffuse = grassCol * rockCover + rockCol * (1 - rockCover);

	float d = distance(worldPixel.xyz, cameraPosition);
	float fogAmount = clamp((d - 100) / 150, 0, 1);
	vec3 bot = vec3(146, 220, 186) / 255.0;

	FragColor = vec4(lerp(diffuse * light, bot, fogAmount), 1);
	//FragColor = vec4(rockCover, rockCover, rockCover, 1);

	//vec3 viewDir = normalize(worldPixel.xyz - cameraPosition);

	//vec3 lightReflect = normalize(reflect(-lightDir, normal));

	//float specular = pow(max(dot(lightReflect, viewDir), 0.0f), 128);

	//FragColor = col * light + specular * col;
}