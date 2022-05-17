#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;

in vec4 worldPixel;

uniform sampler2D diffuse;

void main() 
{
	vec3 camPos = vec3(2, 1, -2);
	vec3 lightDir = vec3(-1, -1, 0);

	vec3 viewDir = normalize(worldPixel.xyz - camPos);

	vec3 lightReflect = normalize(reflect(-lightDir, normal));

	float specular = pow(max(dot(lightReflect, viewDir), 0.0f), 128);
	float light = max(dot(-lightDir, normal), 0.25f);

	vec4 col = texture(diffuse, uv);

	FragColor = col * light + specular * col;
}