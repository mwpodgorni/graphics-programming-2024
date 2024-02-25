#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

// (todo) 03.X: Add the out variables here
uniform mat4 WorldMatrix;

// (todo) 03.X: Add the uniforms here
uniform mat4 ProjectionMatrix;
out vec3 Normal;

void main()
{
	Normal = (WorldMatrix * vec4(VertexNormal, 0.0)).xyz;
	gl_Position = ProjectionMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}
