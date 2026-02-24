/*
   Hyllian's xBR-lv3 Shader
   
   Copyright (C) 2011-2015 Hyllian - sergiogdb@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.


   Incorporates some of the ideas from SABR shader. Thanks to Joshua Street.
*/

uniform float XBR_Y_WEIGHT;
uniform float XBR_EQ_THRESHOLD;
uniform float XBR_EQ_THRESHOLD2;
uniform float XBR_LV2_COEFFICIENT;

uniform sampler2D tex;
uniform vec2 texSize;

varying vec2 texCoord;
varying vec4 TEX1;
varying vec4 TEX2;
varying vec4 TEX3;
varying vec4 TEX4;
varying vec4 TEX5;
varying vec4 TEX6;
varying vec4 TEX7;

const mat3 yuv = mat3(
	0.299,  0.587,  0.114,
   -0.169, -0.331,  0.499,
	0.499, -0.418, -0.0813
);

// keep same delta
const vec4 delta = vec4(0.4, 0.4, 0.4, 0.4);

vec4 df4(vec4 A, vec4 B) {
	return abs(A - B);
}

float c_df(vec3 c1, vec3 c2) {
	vec3 d = abs(c1 - c2);
	return d.r + d.g + d.b;
}

bvec4 eq4(vec4 A, vec4 B) {
	return lessThan(df4(A, B), vec4(XBR_EQ_THRESHOLD));
}

bvec4 eq24(vec4 A, vec4 B) {
	return lessThan(df4(A, B), vec4(XBR_EQ_THRESHOLD2));
}

bvec4 and4(bvec4 A, bvec4 B) {
	return bvec4(A.x && B.x, A.y && B.y, A.z && B.z, A.w && B.w);
}

bvec4 or4(bvec4 A, bvec4 B) {
	return bvec4(A.x || B.x, A.y || B.y, A.z || B.z, A.w || B.w);
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h) {
	return (df4(a,b) + df4(a,c) + df4(d,e) + df4(d,f) + 4.0 * df4(g,h));
}

void main() {
	bvec4 edr, edr_left, edr_up, edr3_left, edr3_up, px;
	bvec4 interp_restriction_lv1, interp_restriction_lv2_left, interp_restriction_lv2_up;
	bvec4 interp_restriction_lv3_left, interp_restriction_lv3_up;
	bvec4 nc, nc30, nc60, nc45, nc15, nc75;
	vec4 fx, fx_left, fx_up, fx3_left, fx3_up;
	vec3 res1, res2, pix1, pix2;
	float blend1, blend2;

	// IMPORTANT: initialize to avoid undefined reads on some compilers
	pix1 = vec3(0.0);
	pix2 = vec3(0.0);
	blend1 = 0.0;
	blend2 = 0.0;

	vec2 fp = fract(texCoord * texSize);

	vec3 A1 = texture2D(tex, TEX1.xw).rgb;
	vec3 B1 = texture2D(tex, TEX1.yw).rgb;
	vec3 C1 = texture2D(tex, TEX1.zw).rgb;

	vec3 A  = texture2D(tex, TEX2.xw).rgb;
	vec3 B  = texture2D(tex, TEX2.yw).rgb;
	vec3 C  = texture2D(tex, TEX2.zw).rgb;

	vec3 D  = texture2D(tex, TEX3.xw).rgb;
	vec3 E  = texture2D(tex, TEX3.yw).rgb;
	vec3 F  = texture2D(tex, TEX3.zw).rgb;

	vec3 G  = texture2D(tex, TEX4.xw).rgb;
	vec3 H  = texture2D(tex, TEX4.yw).rgb;
	vec3 I  = texture2D(tex, TEX4.zw).rgb;

	vec3 G5 = texture2D(tex, TEX5.xw).rgb;
	vec3 H5 = texture2D(tex, TEX5.yw).rgb;
	vec3 I5 = texture2D(tex, TEX5.zw).rgb;

	vec3 A0 = texture2D(tex, TEX6.xy).rgb;
	vec3 D0 = texture2D(tex, TEX6.xz).rgb;
	vec3 G0 = texture2D(tex, TEX6.xw).rgb;

	vec3 C4 = texture2D(tex, TEX7.xy).rgb;
	vec3 F4 = texture2D(tex, TEX7.xz).rgb;
	vec3 I4 = texture2D(tex, TEX7.xw).rgb;

	// Replace mat4x3 + transpose with explicit dot products (GLSL ES 1.00 safe)
	// Original: transpose(mat4x3(...)) * (XBR_Y_WEIGHT * yuv[0])
	vec3 Yw = (XBR_Y_WEIGHT * yuv[0]);

	vec4 b = vec4(dot(B,  Yw), dot(D,  Yw), dot(H,  Yw), dot(F,  Yw));
	vec4 c = vec4(dot(C,  Yw), dot(A,  Yw), dot(G,  Yw), dot(I,  Yw));
	vec4 e = vec4(dot(E,  Yw), dot(E,  Yw), dot(E,  Yw), dot(E,  Yw));
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4v = vec4(dot(I4, Yw), dot(C1, Yw), dot(A0, Yw), dot(G5, Yw));
	vec4 i5v = vec4(dot(I5, Yw), dot(C4, Yw), dot(A1, Yw), dot(G0, Yw));
	vec4 h5v = vec4(dot(H5, Yw), dot(F4, Yw), dot(B1, Yw), dot(D0, Yw));
	vec4 f4v = h5v.yzwx;

	vec4 c1 = i4v.yzwx;
	vec4 g0 = i5v.wxyz;
	vec4 b1 = h5v.zwxy;
	vec4 d0 = h5v.wxyz;

	// same constants as original
	vec4 Ao = vec4( 1.0, -1.0, -1.0,  1.0 );
	vec4 Bo = vec4( 1.0,  1.0, -1.0, -1.0 );
	vec4 Co = vec4( 1.5,  0.5, -0.5,  0.5 );
	vec4 Ax = vec4( 1.0, -1.0, -1.0,  1.0 );
	vec4 Bx = vec4( 0.5,  2.0, -0.5, -2.0 );
	vec4 Cx = vec4( 1.0,  1.0, -0.5,  0.0 );
	vec4 Ay = vec4( 1.0, -1.0, -1.0,  1.0 );
	vec4 By = vec4( 2.0,  0.5, -2.0, -0.5 );
	vec4 Cy = vec4( 2.0,  0.0, -1.0,  0.5 );

	vec4 Az = vec4( 6.0, -2.0, -6.0,  2.0 );
	vec4 Bz = vec4( 2.0,  6.0, -2.0, -6.0 );
	vec4 Cz = vec4( 5.0,  3.0, -3.0, -1.0 );
	vec4 Aw = vec4( 2.0, -6.0, -2.0,  6.0 );
	vec4 Bw = vec4( 6.0,  2.0, -6.0, -2.0 );
	vec4 Cw = vec4( 5.0, -1.0, -3.0,  3.0 );

	fx       = (Ao * fp.y + Bo * fp.x);
	fx_left  = (Ax * fp.y + Bx * fp.x);
	fx_up    = (Ay * fp.y + By * fp.x);
	fx3_left = (Az * fp.y + Bz * fp.x);
	fx3_up   = (Aw * fp.y + Bw * fp.x);

	// Keep CORNER_* behavior EXACTLY. Default path is CORNER_C branch (the else).
#ifdef CORNER_A
	interp_restriction_lv1 = and4(notEqual(e, f), notEqual(e, h));
#elif defined(CORNER_B)
	interp_restriction_lv1 = ((e!=f) && (e!=h)  &&  ( !eq4(f,b) && !eq4(h,d) || eq4(e,i) && !eq4(f,i4v) && !eq4(h,i5v) || eq4(e,g) || eq4(e,c) ) );
#elif defined(CORNER_D)
	interp_restriction_lv1 = ((e!=f) && (e!=h)  &&  ( !eq4(f,b) && !eq4(h,d) || eq4(e,i) && !eq4(f,i4v) && !eq4(h,i5v) || eq4(e,g) || eq4(e,c) ) && (f!=f4v && f!=i || h!=h5v && h!=i || h!=g || f!=c || eq4(b,c1) && eq4(d,g0)));
#else
	interp_restriction_lv1 = and4(
		and4(notEqual(e, f), notEqual(e, h)),
		or4(
			or4(
				and4(not(eq4(f,b)), not(eq4(f,c))),
				and4(not(eq4(h,d)), not(eq4(h,g)))
			),
			or4(
				and4(
					eq4(e,i),
					or4(
						and4(not(eq4(f,f4v)), not(eq4(f,i4v))),
						and4(not(eq4(h,h5v)), not(eq4(h,i5v)))
					)
				),
				or4(eq4(e,g), eq4(e,c))
			)
		)
	);
#endif

	interp_restriction_lv2_left = and4(notEqual(e, g), notEqual(d, g));
	interp_restriction_lv2_up   = and4(notEqual(e, c), notEqual(b, c));
	interp_restriction_lv3_left = and4(eq24(g, g0), not(eq24(d0, g0)));
	interp_restriction_lv3_up   = and4(eq24(c, c1), not(eq24(b1, c1)));

	vec4 fx45 = smoothstep(Co - delta, Co + delta, fx);
	vec4 fx30 = smoothstep(Cx - delta, Cx + delta, fx_left);
	vec4 fx60 = smoothstep(Cy - delta, Cy + delta, fx_up);
	vec4 fx15 = smoothstep(Cz - delta, Cz + delta, fx3_left);
	vec4 fx75 = smoothstep(Cw - delta, Cw + delta, fx3_up);

	edr = and4(
		lessThan(
			weighted_distance(e, c, g, i, h5v, f4v, h, f),
			weighted_distance(h, d, i5v, f, i4v, b, e, i)
		),
		interp_restriction_lv1
	);

	edr_left = and4(lessThanEqual((XBR_LV2_COEFFICIENT * df4(f,g)), df4(h,c)), interp_restriction_lv2_left);
	edr_up   = and4(greaterThanEqual(df4(f,g), (XBR_LV2_COEFFICIENT * df4(h,c))), interp_restriction_lv2_up);
	edr3_left = interp_restriction_lv3_left;
	edr3_up   = interp_restriction_lv3_up;

	nc45 = and4(edr, bvec4(fx45));
	nc30 = and4(edr, and4(edr_left, bvec4(fx30)));
	nc60 = and4(edr, and4(edr_up,   bvec4(fx60)));
	nc15 = and4(and4(edr, edr_left), and4(edr3_left, bvec4(fx15)));
	nc75 = and4(and4(edr, edr_up),   and4(edr3_up,   bvec4(fx75)));

	px = lessThanEqual(df4(e, f), df4(e, h));

	nc = bvec4(
		(nc75.x || nc15.x || nc30.x || nc60.x || nc45.x),
		(nc75.y || nc15.y || nc30.y || nc60.y || nc45.y),
		(nc75.z || nc15.z || nc30.z || nc60.z || nc45.z),
		(nc75.w || nc15.w || nc30.w || nc60.w || nc45.w)
	);

	vec4 final45 = vec4(nc45) * fx45;
	vec4 final30 = vec4(nc30) * fx30;
	vec4 final60 = vec4(nc60) * fx60;
	vec4 final15 = vec4(nc15) * fx15;
	vec4 final75 = vec4(nc75) * fx75;

	vec4 maximo = max(max(max(final15, final75), max(final30, final60)), final45);

	if (nc.x) { pix1 = px.x ? F : H; blend1 = maximo.x; }
	else if (nc.y) { pix1 = px.y ? B : F; blend1 = maximo.y; }
	else if (nc.z) { pix1 = px.z ? D : B; blend1 = maximo.z; }
	else if (nc.w) { pix1 = px.w ? H : D; blend1 = maximo.w; }

	if (nc.w) { pix2 = px.w ? H : D; blend2 = maximo.w; }
	else if (nc.z) { pix2 = px.z ? D : B; blend2 = maximo.z; }
	else if (nc.y) { pix2 = px.y ? B : F; blend2 = maximo.y; }
	else if (nc.x) { pix2 = px.x ? F : H; blend2 = maximo.x; }

	res1 = mix(E, pix1, blend1);
	res2 = mix(E, pix2, blend2);

	vec3 res = mix(res1, res2, step(c_df(E, res1), c_df(E, res2)));

	gl_FragColor = vec4(res, 1.0);
}