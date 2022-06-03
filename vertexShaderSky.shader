#version 330 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vcolor;
layout(location = 2) in vec2 vuv;
layout(location = 3) in vec3 vnormal;

uniform mat4 world, view, projection;

out vec4 worldPixel;

void main() 
{
	worldPixel = world * vec4(vpos, 1.0);
	gl_Position = projection * view * world * vec4(vpos, 1.0);
}