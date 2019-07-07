uniform sampler2D texture;
uniform sampler2D mask;
uniform vec4 lightcol;
uniform vec2 lightpos;

void main()
{
  vec4 color = texture2D(texture,gl_TexCoord[0].xy);
  vec2 invertedGL = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

  float distance = distance(gl_FragCoord.xy, lightpos);
  vec3 lightAttenuation = vec3(0.10, 0.001, 0.000005);

  float attenuation=(lightAttenuation.x+lightAttenuation.y*distance+lightAttenuation.z*distance*distance);
  vec4 alpha=vec4(0.1,0.1,0.1,0.1);

  vec4 lightingTexture=lightcol/attenuation*alpha;

  color += lightingTexture*texture2D(mask, invertedGL).x;

  gl_FragColor = color;
}
