uniform sampler2D texture;
uniform sampler2D mask;
uniform vec4 lightcol;
uniform vec2 lightpos;

void main()
{
  vec4 color = texture2D(texture,gl_TexCoord[0].xy);
  vec2 invertedGL = vec2(gl_TexCoord[0].x, 1.0-gl_TexCoord[0].y);

  float distance = distance(gl_FragCoord.xy, lightpos);
  vec3 lightAttenuation = vec3(0.1, 0.0005, 0.000005);

  float attenuation=(lightAttenuation.x+lightAttenuation.y*distance+lightAttenuation.z*distance*distance);
  vec4 alpha=vec4(0.1,0.1,0.1,0.1);

  vec4 lightingTexture=lightcol/attenuation*alpha;

  float blur_radius = 0.007;

  vec2 offx = vec2(blur_radius, 0.0);
  vec2 offy = vec2(0.0, blur_radius);

  vec4 pixel = texture2D(mask, invertedGL)               * 4.0 +
                 texture2D(mask, invertedGL - offx)        * 2.0 +
                 texture2D(mask, invertedGL + offx)        * 2.0 +
                 texture2D(mask, invertedGL - offy)        * 2.0 +
                 texture2D(mask, invertedGL + offy)        * 2.0 +
                 texture2D(mask, invertedGL - offx - offy) * 1.0 +
                 texture2D(mask, invertedGL - offx + offy) * 1.0 +
                 texture2D(mask, invertedGL + offx - offy) * 1.0 +
                 texture2D(mask, invertedGL + offx + offy) * 1.0 +

                 texture2D(mask, invertedGL - offx*2.0 - offy) * 1.0 +
                 texture2D(mask, invertedGL - offx*2.0 + offy) * 1.0 +
                 texture2D(mask, invertedGL + offx*2.0 + offy) * 1.0 +
                 texture2D(mask, invertedGL + offx*2.0 - offy) * 1.0 +

                 texture2D(mask, invertedGL - offx - offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL - offx + offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL + offx + offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL + offx - offy*2.0) * 1.0 +

                 texture2D(mask, invertedGL - offx*2.0 - offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL - offx*2.0 + offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL + offx*2.0 + offy*2.0) * 1.0 +
                 texture2D(mask, invertedGL + offx*2.0 - offy*2.0) * 1.0;

  color += lightingTexture*(pixel.x/28.0);
  //color = (pixel/16.0);

  gl_FragColor = color;
}
