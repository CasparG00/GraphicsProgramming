#version 330 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vcolor;
layout(location = 2) in vec2 vuv;
layout(location = 3) in vec3 vnormal;

uniform mat4 world, view, projection;
uniform sampler2D heightmap;

out vec3 color;
out vec2 uv;
out vec3 normal;
out vec4 worldPixel;

void main() 
{
	worldPixel = world * vec4(vpos, 1.0);

	vec4 diffuse = texture(heightmap, vuv);
	worldPixel.y += diffuse.r * 100;

	gl_Position = projection * view * worldPixel;
	color = vcolor;
	uv = vuv;
	normal = mat3(world) * vnormal;
}