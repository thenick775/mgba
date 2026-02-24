varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;

float wiiuLut31(float x) {
	int i = int(floor(x * 31.0 + 0.5));

	if (i < 0) i = 0;
	if (i > 31) i = 31;

	if (i == 0)  return   0.0 / 255.0;
	if (i == 1)  return   6.0 / 255.0;
	if (i == 2)  return  12.0 / 255.0;
	if (i == 3)  return  18.0 / 255.0;
	if (i == 4)  return  24.0 / 255.0;
	if (i == 5)  return  31.0 / 255.0;
	if (i == 6)  return  37.0 / 255.0;
	if (i == 7)  return  43.0 / 255.0;
	if (i == 8)  return  49.0 / 255.0;
	if (i == 9)  return  55.0 / 255.0;
	if (i == 10) return  61.0 / 255.0;
	if (i == 11) return  67.0 / 255.0;
	if (i == 12) return  73.0 / 255.0;
	if (i == 13) return  79.0 / 255.0;
	if (i == 14) return  86.0 / 255.0;
	if (i == 15) return  92.0 / 255.0;
	if (i == 16) return  98.0 / 255.0;
	if (i == 17) return 104.0 / 255.0;
	if (i == 18) return 111.0 / 255.0;
	if (i == 19) return 117.0 / 255.0;
	if (i == 20) return 123.0 / 255.0;
	if (i == 21) return 129.0 / 255.0;
	if (i == 22) return 135.0 / 255.0;
	if (i == 23) return 141.0 / 255.0;
	if (i == 24) return 148.0 / 255.0;
	if (i == 25) return 154.0 / 255.0;
	if (i == 26) return 159.0 / 255.0;
	if (i == 27) return 166.0 / 255.0;
	if (i == 28) return 172.0 / 255.0;
	if (i == 29) return 178.0 / 255.0;
	if (i == 30) return 184.0 / 255.0;
	// i == 31
	return 191.0 / 255.0;
}

void main() {
	vec4 color = texture2D(tex, texCoord);

	color.r = wiiuLut31(color.r);
	color.g = wiiuLut31(color.g);
	color.b = wiiuLut31(color.b);

	gl_FragColor = color;
}
