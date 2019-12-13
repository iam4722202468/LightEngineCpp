uniform sampler2D texture;
uniform sampler2D base;
uniform vec2 offset;

void main()
{
  // Now we need a third part, the location of the light
  // Use that to calculate the angle the light source hits the object
  // Cos and Sin that to figure out how to light each color

  vec4 pixel = texture2D(texture,gl_TexCoord[0].xy/512.0);
  vec4 pixelBase = texture2D(base,(gl_TexCoord[0].xy - offset)/(vec2(1000.0, 800.0)));
  gl_FragColor = pixelBase*pixel;

}
