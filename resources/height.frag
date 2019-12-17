uniform vec2 lightpos;
uniform vec2 offset;

void main()
{
  vec4 color = gl_Color;

  vec4 vertex = gl_Vertex;
  vertex.x += lightpos.x * (color.r * 100.0);
  vertex.y += lightpos.y;

  vertex.x -= offset.x;
  vertex.y -= offset.y;

  gl_Position = gl_ModelViewProjectionMatrix * vertex;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  gl_FrontColor = gl_Color*vec4(1.0,0.0,0.0,1.0);
}
