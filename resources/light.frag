uniform sampler2D texture;
uniform sampler2D mask;
uniform sampler2D light;
uniform vec4 lightcol;
uniform vec2 lightpos;
uniform float distanceScale;

uniform sampler2D normals;
uniform sampler2D sprites;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
  vec4 color = texture2D(texture,gl_TexCoord[0].xy);

  vec4 normalcol = texture2D(normals,gl_TexCoord[0].xy);
  vec4 spritecol = texture2D(sprites,gl_TexCoord[0].xy);

  float distance = distance(gl_FragCoord.xy, lightpos);
  distance += rand(gl_TexCoord[0].x * gl_TexCoord[0].y * lightpos) * distance/20.0;
  distance *= distanceScale;

  vec4 shadows = texture2D(mask, gl_TexCoord[0].xy);

  vec4 lightBrightness = texture2D(light, vec2(distance,0.0)/1000.0);

  if (normalcol.a == 0.0) {
    color += shadows.x*lightcol*lightBrightness.a;
    gl_FragColor = color;
  } else {
    float amountx = (lightpos.x - gl_FragCoord.x)/distance;
    float amounty = (lightpos.y - gl_FragCoord.y)/distance;

    color += lightBrightness.a*lightcol*vec4(1.5,1.5,1.5,1.0)*amountx*20.0*normalcol.r*spritecol;
    color += lightBrightness.a*lightcol*vec4(1.5,1.5,1.5,1.0)*amounty*20.0*normalcol.b*spritecol;

    color += lightBrightness.a*lightcol*vec4(1.5,1.5,1.5,1.0)*amountx*20.0*(-normalcol.b)*spritecol;
    color += lightBrightness.a*lightcol*vec4(1.5,1.5,1.5,1.0)*amounty*20.0*(-normalcol.g)*spritecol;
    gl_FragColor = color;
  }
}
