varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 outputSize;

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

void main() {
  vec3 c = texture2D(tex, texCoord).rgb;

  // gamma-ish shaping the original shader intended
  c = pow(c * vec3(0.8), vec3(1.8)) + vec3(0.16);

  // Stripe mask aligned to OUTPUT pixels (stable across scale factors)
  float x = mod(floor(gl_FragCoord.x), 3.0);
  vec3 maskX =
    (x < 0.5) ? vec3(0.2, 0.2, 1.0) :
    (x < 1.5) ? vec3(0.2, 1.0, 0.2) :
                vec3(1.0, 0.2, 0.2);

  float y = mod(floor(gl_FragCoord.y), 4.0);
  vec3 maskY = (y < 3.5) ? vec3(1.0) : vec3(0.8);

  c *= (maskX * maskY);

  gl_FragColor = vec4(c, 1.0);
}
