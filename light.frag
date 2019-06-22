uniform sampler2D texture;

float minimum_distance(vec2 v, vec2 w, vec2 p) {
  float l2 = length(v - w)*length(v - w);  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) return distance(p, v);   // v == w case
  float t = max(0.0, min(1.0, dot(p - v, w - v) / l2));
  vec2 projection = v + t * (w - v);  // Projection falls on the segment
  return distance(p, projection);
}

bool intersect(vec2 start, vec2 end, vec2 lineStart, vec2 lineEnd) {
  float p0_x = start.x;
  float p0_y = start.y;
  float p1_x = end.x;
  float p1_y = end.y;
  float p2_x = lineStart.x;
  float p2_y = lineStart.y;
  float p3_x = lineEnd.x;
  float p3_y = lineEnd.y;

  float s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

  float s, t;
  s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
  t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

  if (s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0)
    return false;

  return true;
}

void main()
{
  vec2 lights[14];
  lights[0] = vec2(800.0,300.0);
  lights[1] = vec2(300.0,800.0);
  lights[2] = vec2(200.0,400.0);
  lights[3] = vec2(500.0,500.0);
  lights[4] = vec2(500.0,800.0);
  lights[5] = vec2(1000.0,800.0);
  lights[6] = vec2(300.0,100.0);
  lights[7] = vec2(800.0,300.0);
  lights[8] = vec2(300.0,800.0);
  lights[9] = vec2(200.0,400.0);
  lights[10] = vec2(500.0,500.0);
  lights[11] = vec2(500.0,800.0);
  lights[12] = vec2(1000.0,800.0);
  lights[13] = vec2(300.0,100.0);

  vec4 lightColors[14];
  lightColors[0] = vec4(0.0,100.0,100.0,100.0);
  lightColors[1] = vec4(100.0,0.0,100.0,0.0);
  lightColors[2] = vec4(100.0,100.0,100.0,0.0);
  lightColors[3] = vec4(100.0,0.0,50.0,50.0);
  lightColors[4] = vec4(100.0,0.0,100.0,0.0);
  lightColors[5] = vec4(100.0,0.0,100.0,0.0);
  lightColors[6] = vec4(100.0,50.0,100.0,0.0);
  lightColors[7] = vec4(0.0,100.0,100.0,100.0);
  lightColors[8] = vec4(100.0,0.0,100.0,0.0);
  lightColors[9] = vec4(100.0,100.0,100.0,0.0);
  lightColors[10] = vec4(100.0,0.0,50.0,50.0);
  lightColors[11] = vec4(100.0,0.0,100.0,0.0);
  lightColors[12] = vec4(100.0,0.0,100.0,0.0);
  lightColors[13] = vec4(100.0,50.0,100.0,0.0);

  vec2 d = gl_TexCoord[0].xy;

  vec2 lines[12];
  lines[0] = vec2(0.5,0.5);
  lines[1] = vec2(0.1,0.1);
  lines[2] = vec2(0.1,0.1);
  lines[3] = vec2(0.1,0.4);
  lines[4] = vec2(0.1,0.4);
  lines[5] = vec2(0.5,0.5);
  lines[6] = vec2(0.7,0.7);
  lines[7] = vec2(0.3,0.3);
  lines[8] = vec2(0.3,0.3);
  lines[9] = vec2(0.3,0.6);
  lines[10] = vec2(0.3,0.6);
  lines[11] = vec2(0.7,0.7);

  vec4 color = texture2D(texture,gl_TexCoord[0].xy);
  vec3 lightAttenuation = vec3(200, 5.0, 0.001);

  for (int x = 0; x < 14; ++x) {
    int intersections = 0;

    vec4 lightcol = lightColors[x];
    vec2 lightpos = lights[x];
    vec2 light = vec2(lightpos.x/1000.0, lightpos.y/800.0);

    if (distance(light, d) < 1.5) {
      float distance = distance(gl_FragCoord.xy, -lightpos + vec2(lightpos.x*2.0, 800.0));
      float attenuation=(lightAttenuation.x+lightAttenuation.y*distance+lightAttenuation.z*distance*distance);  
      vec4 lightingTexture=lightcol/attenuation;

      for (int y = 0; y < 5; y += 2) {
        vec2 a = lines[y];
        vec2 b = lines[y+1];

        if (!intersect(a,b,d,light)) {
          intersections += 1;
        }
      }
      if (intersections < 2)
        color += lightingTexture;
    }

  }


  gl_FragColor = color;
}
