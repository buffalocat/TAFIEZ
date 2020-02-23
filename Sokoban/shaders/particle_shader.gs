#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in Vertex
{
  vec2 TexOffset;
  vec2 Scale;
  vec4 Color;
} vertex[];


out vec2 TexCoord;
out vec4 Color;

uniform float texScale;
uniform mat4 Proj;

void main()
{
  vec4 Q = gl_in[0].gl_Position;
  vec4 P = Proj * Q;
  vec2 s = vertex[0].Scale;
  Color = vertex[0].Color;
  
	gl_Position = vec4(P.x - s.x, P.y - s.y, P.zw);
  TexCoord = vertex[0].TexOffset;
  EmitVertex();
  
  gl_Position = vec4(P.x - s.x, P.y + s.y, P.zw);
  TexCoord = vertex[0].TexOffset + vec2(0, texScale);
  EmitVertex();
  
  gl_Position = vec4(P.x + s.x, P.y - s.y, P.zw);
  TexCoord = vertex[0].TexOffset + vec2(texScale, 0);
  EmitVertex();
  
  gl_Position = vec4(P.x + s.x, P.y + s.y, P.zw);
  TexCoord = vertex[0].TexOffset + vec2(texScale, texScale);
  EmitVertex();
  
  EndPrimitive();
}
