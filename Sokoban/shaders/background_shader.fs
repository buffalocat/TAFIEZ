#version 330 core
in float z;

out vec4 FragColor;

uniform vec4 color_down;
uniform vec4 color_up;

void main()
{
  FragColor = ((-z+1) * color_down + (z+1) * color_up) / 2;
}
