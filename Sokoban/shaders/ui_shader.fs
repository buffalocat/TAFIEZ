#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D atlas;
uniform vec4 color;

void main()
{
  FragColor = color * texture(atlas, TexCoord);
}
