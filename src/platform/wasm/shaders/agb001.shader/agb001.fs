varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;

const vec3 kArrayX0 = vec3(0.2, 0.2, 1.0);
const vec3 kArrayX1 = vec3(0.2, 1.0, 0.2);
const vec3 kArrayX2 = vec3(1.0, 0.2, 0.2);
const vec3 kArrayY0 = vec3(1.0, 1.0, 1.0);
const vec3 kArrayY1 = vec3(1.0, 1.0, 1.0);
const vec3 kArrayY2 = vec3(1.0, 1.0, 1.0);
const vec3 kArrayY3 = vec3(0.8, 0.8, 0.8);


void main() {
	vec4 color = texture2D(tex, texCoord);
	// WebGL1-safe: precomputed constant LUTs (no runtime array writes)
	color.rgb = pow(color.rgb * vec3(0.8, 0.8, 0.8), vec3(1.8, 1.8, 1.8)) + vec3(0.16, 0.16, 0.16);
	int xi = int(mod(texCoord.s * texSize.x * 3.0, 3.0));
	vec3 ax = (xi == 0) ? kArrayX0 : ((xi == 1) ? kArrayX1 : kArrayX2);
	int yi = int(mod(texCoord.t * texSize.y * 4.0, 4.0));
	vec3 ay = (yi == 0) ? kArrayY0 : ((yi == 1) ? kArrayY1 : ((yi == 2) ? kArrayY2 : kArrayY3));
	color.rgb *= ax;
	color.rgb *= ay;
	color.a = 0.5;
	gl_FragColor = color;
}
