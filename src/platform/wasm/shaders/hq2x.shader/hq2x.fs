/* MIT License
*
* Copyright (c) 2015-2023 Lior Halphon
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
/* Based on this (really good) article: http://blog.pkh.me/p/19-butchering-hqx-scaling-filters.html */

/* The colorspace used by the HQnx filters is not really YUV, despite the algorithm description claims it is. It is
   also not normalized. Therefore, we shall call the colorspace used by HQnx "HQ Colorspace" to avoid confusion. */

varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;

float _bit(float v, int i)
{
	// v is an integer value stored as float
	float p = exp2(float(i)); // 2^i, GLSL ES 1.00-friendly
	return floor(mod(floor(v / p), 2.0));
}

bool _matchMask(float pattern, int mask, int ref)
{
	// (pattern & mask) == ref, implemented without bitwise ops.
	float m = float(mask);
	float r = float(ref);
	float mism = 0.0;
	for (int i = 0; i < 8; ++i)
	{
		float mb = _bit(m, i);
		if (mb > 0.5)
		{
			float pb = _bit(pattern, i);
			float rb = _bit(r, i);
			mism += abs(pb - rb);
		}
	}
	return mism < 0.5;
}

vec3 rgb_to_hq_colospace(vec4 rgb)
{
	return vec3(
		0.250 * rgb.r + 0.250 * rgb.g + 0.250 * rgb.b,
		0.250 * rgb.r - 0.000 * rgb.g - 0.250 * rgb.b,
	   -0.125 * rgb.r + 0.250 * rgb.g - 0.125 * rgb.b
	);
}

bool is_different(vec4 a, vec4 b)
{
	vec3 diff = abs(rgb_to_hq_colospace(a) - rgb_to_hq_colospace(b));
	return diff.x > 0.018 || diff.y > 0.002 || diff.z > 0.005;
}

// WebGL1: no bitwise ops in GLSL ES 1.00

vec4 interp_2px(vec4 c1, float w1, vec4 c2, float w2)
{
	return (c1 * w1 + c2 * w2) / (w1 + w2);
}

vec4 interp_3px(vec4 c1, float w1, vec4 c2, float w2, vec4 c3, float w3)
{
	return (c1 * w1 + c2 * w2 + c3 * w3) / (w1 + w2 + w3);
}

vec4 scale(sampler2D image, vec2 position, vec2 input_resolution)
{
	// o = offset, the width of a pixel
	vec2 o = vec2(1.0, 1.0) / input_resolution;

	/* We always calculate the top left pixel.  If we need a different pixel, we flip the image */

	// p = the position within a pixel [0...1]
	vec2 p = fract(position * input_resolution);

	if (p.x > 0.5) o.x = -o.x;
	if (p.y > 0.5) o.y = -o.y;

	vec4 w0 = texture2D(image, position + vec2( -o.x, -o.y));
	vec4 w1 = texture2D(image, position + vec2(  0.0, -o.y));
	vec4 w2 = texture2D(image, position + vec2(  o.x, -o.y));
	vec4 w3 = texture2D(image, position + vec2( -o.x,  0.0));
	vec4 w4 = texture2D(image, position + vec2(  0.0,  0.0));
	vec4 w5 = texture2D(image, position + vec2(  o.x,  0.0));
	vec4 w6 = texture2D(image, position + vec2( -o.x,  o.y));
	vec4 w7 = texture2D(image, position + vec2(  0.0,  o.y));
	vec4 w8 = texture2D(image, position + vec2(  o.x,  o.y));

	float pattern = 0.0;
	if (is_different(w0, w4)) pattern += 1.0;
	if (is_different(w1, w4)) pattern += 2.0;
	if (is_different(w2, w4)) pattern += 4.0;
	if (is_different(w3, w4)) pattern += 8.0;
	if (is_different(w5, w4)) pattern += 16.0;
	if (is_different(w6, w4)) pattern += 32.0;
	if (is_different(w7, w4)) pattern += 64.0;
	if (is_different(w8, w4)) pattern += 128.0;

	// NOTE: hex literals (0x..) are not GLSL ES 1.00. Converted to decimal.

	if ((_matchMask(pattern, 191, 55) || _matchMask(pattern, 219, 19)) && is_different(w1, w5))
	{
		return interp_2px(w4, 3.0, w3, 1.0);
	}
	if ((_matchMask(pattern, 219, 73) || _matchMask(pattern, 239, 109)) && is_different(w7, w3))
	{
		return interp_2px(w4, 3.0, w1, 1.0);
	}
	if ((_matchMask(pattern, 11, 11) || _matchMask(pattern, 254, 74) || _matchMask(pattern, 254, 26)) && is_different(w3, w1))
	{
		return w4;
	}
	if ((_matchMask(pattern, 111, 42) || _matchMask(pattern, 91, 10) || _matchMask(pattern, 191, 58) || _matchMask(pattern, 223, 90) ||
		 _matchMask(pattern, 159, 138) || _matchMask(pattern, 207, 138) || _matchMask(pattern, 239, 78) || _matchMask(pattern, 63, 14) ||
		 _matchMask(pattern, 251, 90) || _matchMask(pattern, 187, 138) || _matchMask(pattern, 127, 90) || _matchMask(pattern, 175, 138) ||
		 _matchMask(pattern, 235, 138)) && is_different(w3, w1))
	{
		return interp_2px(w4, 3.0, w0, 1.0);
	}
	if (_matchMask(pattern, 11, 8))
	{
		return interp_3px(w4, 2.0, w0, 1.0, w1, 1.0);
	}
	if (_matchMask(pattern, 11, 2))
	{
		return interp_3px(w4, 2.0, w0, 1.0, w3, 1.0);
	}
	if (_matchMask(pattern, 47, 47))
	{
		return interp_3px(w4, 4.0, w3, 1.0, w1, 1.0);
	}
	if (_matchMask(pattern, 191, 55) || _matchMask(pattern, 219, 19))
	{
		return interp_3px(w4, 5.0, w1, 2.0, w3, 1.0);
	}
	if (_matchMask(pattern, 219, 73) || _matchMask(pattern, 239, 109))
	{
		return interp_3px(w4, 5.0, w3, 2.0, w1, 1.0);
	}
	if (_matchMask(pattern, 27, 3) || _matchMask(pattern, 79, 67) || _matchMask(pattern, 139, 131) || _matchMask(pattern, 107, 67))
	{
		return interp_2px(w4, 3.0, w3, 1.0);
	}
	if (_matchMask(pattern, 75, 9) || _matchMask(pattern, 139, 137) || _matchMask(pattern, 31, 25) || _matchMask(pattern, 59, 25))
	{
		return interp_2px(w4, 3.0, w1, 1.0);
	}
	if (_matchMask(pattern, 126, 42) || _matchMask(pattern, 239, 171) || _matchMask(pattern, 191, 143) || _matchMask(pattern, 126, 14))
	{
		return interp_3px(w4, 2.0, w3, 3.0, w1, 3.0);
	}
	if (_matchMask(pattern, 251, 106) || _matchMask(pattern, 111, 110) || _matchMask(pattern, 63, 62) || _matchMask(pattern, 251, 250) ||
		_matchMask(pattern, 223, 222) || _matchMask(pattern, 223, 30))
	{
		return interp_2px(w4, 3.0, w0, 1.0);
	}
	if (_matchMask(pattern, 10, 0) || _matchMask(pattern, 79, 75) || _matchMask(pattern, 159, 27) || _matchMask(pattern, 47, 11) ||
		_matchMask(pattern, 190, 10) || _matchMask(pattern, 238, 10) || _matchMask(pattern, 126, 10) || _matchMask(pattern, 235, 75) ||
		_matchMask(pattern, 59, 27))
	{
		return interp_3px(w4, 2.0, w3, 1.0, w1, 1.0);
	}

	return interp_3px(w4, 6.0, w3, 1.0, w1, 1.0);
}

void main()
{
	gl_FragColor = scale(tex, texCoord, texSize);
}
