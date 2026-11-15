varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;

void main() {
	vec4 color = texture2D(tex, texCoord);
	color.rgb = pow(color.rgb * vec3(0.8, 0.8, 0.8), vec3(1.8, 1.8, 1.8)) + vec3(0.16, 0.16, 0.16);

	// GLSL ES 1.00 cannot dynamically index locally written arrays.
	float x = mod(texCoord.s * texSize.x * 3.0, 3.0);
	if (x < 1.0) {
		color.rgb *= vec3(0.2, 0.2, 1.0);
	} else if (x < 2.0) {
		color.rgb *= vec3(0.2, 1.0, 0.2);
	} else {
		color.rgb *= vec3(1.0, 0.2, 0.2);
	}

	float y = mod(texCoord.t * texSize.y * 4.0, 4.0);
	if (y >= 3.0) {
		color.rgb *= vec3(0.8, 0.8, 0.8);
	}

	color.a = 0.5;
	gl_FragColor = color;
}
