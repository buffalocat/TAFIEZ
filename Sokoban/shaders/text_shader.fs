#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex;
uniform vec4 color;

void main()
{
  FragColor = vec4(1, 1, 1, texture(tex, TexCoord).r) * color;
}
