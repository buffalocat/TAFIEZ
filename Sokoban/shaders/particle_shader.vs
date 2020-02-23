#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexOffset;
layout (location = 2) in vec2 aScale;
layout (location = 3) in vec4 aColor;

out Vertex
{
  vec2 TexOffset;
  vec2 Scale;
  vec4 Color;
} vertex;

uniform float texScale;
uniform mat4 View;

void main()
{
	gl_Position = View * vec4(aPos, 1);
  vertex.TexOffset = texScale * aTexOffset;
  vertex.Scale = aScale;
	vertex.Color = aColor;
}
