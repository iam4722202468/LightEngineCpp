uniform sampler2D texture;
uniform vec2 textureSize;

void main()
{
  vec4 pixel = texture2D(texture,gl_TexCoord[0].xy/textureSize);
  gl_FragColor = pixel;
}
