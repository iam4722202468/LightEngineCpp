uniform sampler2D texture;
uniform sampler2D mask;
uniform sampler2D light;
uniform vec4 lightcol;
uniform vec2 lightpos;

void main()
{
  vec4 color = texture2D(texture,gl_TexCoord[0].xy);

  float distance = distance(gl_FragCoord.xy, lightpos);

  vec2 invertedGL = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
  vec4 shadows = texture2D(mask, invertedGL);

  if (distance*1.5 < 1000.0) {
    vec4 lightBrightness = texture2D(light, vec2(distance*1.5,0.0)/1000.0);
    vec4 alpha = vec4(1.0,1.0,1.0,0.1);

    color += shadows.x*lightcol*lightBrightness.a/alpha;
  }

  gl_FragColor = color;
}
