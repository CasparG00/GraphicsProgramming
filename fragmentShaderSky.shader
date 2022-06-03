#version 330 core

out vec4 FragColor;

in vec4 worldPixel;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

vec3 lerp(vec3 a, vec3 b, float t)
{
	return a + (b - a) * t;
}

void main() 
{
	vec3 lightDir = normalize(lightDirection);
	vec3 viewDir = normalize(worldPixel.xyz - cameraPosition);

	vec3 top = vec3(71, 125, 133) / 255.0;
	vec3 bot = vec3(146, 220, 186) / 255.0;

	float sun = smoothstep(0.85, 0.9, pow(max(dot(-viewDir, lightDir), 0.0), 32));

	FragColor = vec4(lerp(bot, top, viewDir.y) + sun, 1);
}