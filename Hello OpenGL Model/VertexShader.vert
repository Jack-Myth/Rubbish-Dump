#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTextureCoordinate;
out vec2 pTextureCoordinate;
out vec3 aNormal;
out vec3 PixelPos;
uniform mat4 ModelMatrix,ViewMatrix,ProjectionMatrix;
out mat4x4 aViewMatrix;

void main()
{
	gl_Position= ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(aPos,1.f);
	pTextureCoordinate=vTextureCoordinate;
	aNormal=vNormal;
	PixelPos=(ViewMatrix*ModelMatrix*vec4(aPos,1.f)).xyz;
	aViewMatrix=ViewMatrix;
}