/*
   Hyllian's xBR-lv2 Shader
   
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
uniform float XBR_SCALE;
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

const vec4 Ao = vec4( 1.0, -1.0, -1.0,  1.0 );
const vec4 Bo = vec4( 1.0,  1.0, -1.0, -1.0 );
const vec4 Co = vec4( 1.5,  0.5, -0.5,  0.5 );
const vec4 Ax = vec4( 1.0, -1.0, -1.0,  1.0 );
const vec4 Bx = vec4( 0.5,  2.0, -0.5, -2.0 );
const vec4 Cx = vec4( 1.0,  1.0, -0.5,  0.0 );
const vec4 Ay = vec4( 1.0, -1.0, -1.0,  1.0 );
const vec4 By = vec4( 2.0,  0.5, -2.0, -0.5 );
const vec4 Cy = vec4( 2.0,  0.0, -1.0,  0.5 );
const vec4 Ci = vec4( 0.25, 0.25, 0.25, 0.25 );

const vec3 Y = vec3(0.2126, 0.7152, 0.0722);

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

bvec4 and4(bvec4 A, bvec4 B) {
	return bvec4(A.x && B.x, A.y && B.y, A.z && B.z, A.w && B.w);
}

bvec4 nand4(bvec4 A, bvec4 B) {
	return bvec4(!(A.x && B.x), !(A.y && B.y), !(A.z && B.z), !(A.w && B.w));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h) {
	return (df4(a,b) + df4(a,c) + df4(d,e) + df4(d,f) + 4.0 * df4(g,h));
}

float lum(vec3 rgb) {
	return dot(rgb, Y) * XBR_Y_WEIGHT;
}

void main() {
	// fractional position inside the source pixel (in source-pixel space)
	vec2 fp = fract(texCoord * texSize);

	// Sample neighborhood
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

	// Replace mat4x3/transpose with explicit luma packing (GLSL ES 1.00 safe)
	vec4 b = vec4(lum(B),  lum(D),  lum(H),  lum(F));
	vec4 c = vec4(lum(C),  lum(A),  lum(G),  lum(I));
	vec4 e = vec4(lum(E),  lum(E),  lum(E),  lum(E));
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4v = vec4(lum(I4), lum(C1), lum(A0), lum(G5));
	vec4 i5v = vec4(lum(I5), lum(C4), lum(A1), lum(G0));
	vec4 h5v = vec4(lum(H5), lum(F4), lum(B1), lum(D0));
	vec4 f4v = h5v.yzwx;

	// Edge detection math
	vec4 fx      = (Ao * fp.y + Bo * fp.x);
	vec4 fx_left = (Ax * fp.y + Bx * fp.x);
	vec4 fx_up   = (Ay * fp.y + By * fp.x);

	bvec4 interp_restriction_lv0 = and4(notEqual(e, f), notEqual(e, h));
	bvec4 interp_restriction_lv1 = interp_restriction_lv0;

	bvec4 interp_restriction_lv2_left = and4(notEqual(e, g), notEqual(d, g));
	bvec4 interp_restriction_lv2_up   = and4(notEqual(e, c), notEqual(b, c));

	vec4 delta  = vec4(1.0 / XBR_SCALE);
	vec4 deltaL = vec4(0.5 / XBR_SCALE, 1.0 / XBR_SCALE, 0.5 / XBR_SCALE, 1.0 / XBR_SCALE);
	vec4 deltaU = deltaL.yxwz;

	vec4 fx45i = clamp((fx      + delta  - Co - Ci) / (2.0 * delta ), 0.0, 1.0);
	vec4 fx45  = clamp((fx      + delta  - Co     ) / (2.0 * delta ), 0.0, 1.0);
	vec4 fx30  = clamp((fx_left + deltaL - Co     ) / (2.0 * deltaL), 0.0, 1.0);
	vec4 fx60  = clamp((fx_up   + deltaU - Co     ) / (2.0 * deltaU), 0.0, 1.0);

	vec4 wd1 = weighted_distance(e, c, g, i, h5v, f4v, h, f);
	vec4 wd2 = weighted_distance(h, d, i5v, f, i4v, b, e, i);

	bvec4 edri = and4(lessThanEqual(wd1, wd2), interp_restriction_lv0);
	bvec4 edr  = and4(lessThan(wd1, wd2),      interp_restriction_lv1);

	bvec4 edr_left = and4(
		lessThanEqual((XBR_LV2_COEFFICIENT * df4(f, g)), df4(h, c)),
		interp_restriction_lv2_left
	);
	bvec4 edr_up = and4(
		greaterThanEqual(df4(f, g), (XBR_LV2_COEFFICIENT * df4(h, c))),
		interp_restriction_lv2_up
	);

	edr      = and4(edr, nand4(edri.yzwx, edri.wxyz));
	edr_left = and4(and4(edr_left, edr), eq4(e, c));
	edr_up   = and4(and4(edr_up,   edr), eq4(e, g));

	fx45  *= vec4(edr);
	fx30  *= vec4(edr_left);
	fx60  *= vec4(edr_up);
	fx45i *= vec4(edri);

	vec4 px = vec4(lessThanEqual(df4(e, f), df4(e, h)));

	vec4 maximos = max(max(fx30, fx60), max(fx45, fx45i));

	vec3 res1 = E;
	res1 = mix(res1, mix(H, F, px.x), maximos.x);
	res1 = mix(res1, mix(B, D, px.z), maximos.z);

	vec3 res2 = E;
	res2 = mix(res1, mix(F, B, px.y), maximos.y);
	res2 = mix(res1, mix(D, H, px.w), maximos.w);

	vec3 res = mix(res1, res2, step(c_df(E, res1), c_df(E, res2)));

	gl_FragColor = vec4(res, 1.0);
}