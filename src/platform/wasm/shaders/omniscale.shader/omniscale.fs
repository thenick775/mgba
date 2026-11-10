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
/* OmniScale is derived from the pattern based design of HQnx, but with the following general differences:
	- The actual output calculating was completely redesigned as resolution independent graphic generator. This allows
	  scaling to any factor.
	- HQnx approximations that were good enough for a 2x/3x/4x factor were refined, creating smoother gradients.
	- "Quarters" can be interpolated in more ways than in the HQnx filters
	- If a pattern does not provide enough information to determine the suitable scaling interpolation, up to 16 pixels
	  per quarter are sampled (in contrast to the usual 9) in order to determine the best interpolation.
 */

varying vec2 texCoord;
uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 outputSize;

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
	for (int i = 0; i < 15; ++i)
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

/* We use the same colorspace as the HQ algorithms. */
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

vec4 scale(sampler2D image, vec2 position, vec2 input_resolution, vec2 output_resolution)
{
	// o = offset, the width of a pixel
	vec2 o = vec2(1.0, 1.0) / input_resolution;

	/* We always calculate the top left quarter.  If we need a different quarter, we flip our co-ordinates */

	// p = the position within a pixel [0...1]
	vec2 p = fract(position * input_resolution);

	if (p.x > 0.5)
	{
		o.x = -o.x;
		p.x = 1.0 - p.x;
	}
	if (p.y > 0.5)
	{
		o.y = -o.y;
		p.y = 1.0 - p.y;
	}

	vec4 w0 = texture2D(image, position + vec2(-o.x, -o.y));
	vec4 w1 = texture2D(image, position + vec2( 0.0, -o.y));
	vec4 w2 = texture2D(image, position + vec2( o.x, -o.y));
	vec4 w3 = texture2D(image, position + vec2(-o.x,  0.0));
	vec4 w4 = texture2D(image, position + vec2( 0.0,  0.0));
	vec4 w5 = texture2D(image, position + vec2( o.x,  0.0));
	vec4 w6 = texture2D(image, position + vec2(-o.x,  o.y));
	vec4 w7 = texture2D(image, position + vec2( 0.0,  o.y));
	vec4 w8 = texture2D(image, position + vec2( o.x,  o.y));

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
		return mix(w4, w3, 0.5 - p.x);
	}
	if ((_matchMask(pattern, 219, 73) || _matchMask(pattern, 239, 109)) && is_different(w7, w3))
	{
		return mix(w4, w1, 0.5 - p.y);
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
		return mix(w4, mix(w4, w0, 0.5 - p.x), 0.5 - p.y);
	}
	if (_matchMask(pattern, 11, 8))
	{
		return mix(mix(w0 * 0.375 + w1 * 0.25 + w4 * 0.375, w4 * 0.5 + w1 * 0.5, p.x * 2.0), w4, p.y * 2.0);
	}
	if (_matchMask(pattern, 11, 2))
	{
		return mix(mix(w0 * 0.375 + w3 * 0.25 + w4 * 0.375, w4 * 0.5 + w3 * 0.5, p.y * 2.0), w4, p.x * 2.0);
	}
	if (_matchMask(pattern, 47, 47))
	{
		float dist = length(p - vec2(0.5));
		float pixel_size = length(1.0 / (output_resolution / input_resolution));
		if (dist < 0.5 - pixel_size / 2.0)
		{
			return w4;
		}
		vec4 r;
		if (is_different(w0, w1) || is_different(w0, w3))
		{
			r = mix(w1, w3, p.y - p.x + 0.5);
		}
		else
		{
			r = mix(mix(w1 * 0.375 + w0 * 0.25 + w3 * 0.375, w3, p.y * 2.0), w1, p.x * 2.0);
		}

		if (dist > 0.5 + pixel_size / 2.0)
		{
			return r;
		}
		return mix(w4, r, (dist - 0.5 + pixel_size / 2.0) / pixel_size);
	}
	if (_matchMask(pattern, 191, 55) || _matchMask(pattern, 219, 19))
	{
		float dist = p.x - 2.0 * p.y;
		float pixel_size = length(1.0 / (output_resolution / input_resolution)) * sqrt(5.0);
		if (dist > pixel_size / 2.0)
		{
			return w1;
		}
		vec4 r = mix(w3, w4, p.x + 0.5);
		if (dist < -pixel_size / 2.0)
		{
			return r;
		}
		return mix(r, w1, (dist + pixel_size / 2.0) / pixel_size);
	}
	if (_matchMask(pattern, 219, 73) || _matchMask(pattern, 239, 109))
	{
		float dist = p.y - 2.0 * p.x;
		float pixel_size = length(1.0 / (output_resolution / input_resolution)) * sqrt(5.0);
		if (dist > pixel_size / 2.0)
		{
			return w3;
		}
		vec4 r = mix(w1, w4, p.x + 0.5);
		if (dist < -pixel_size / 2.0)
		{
			return r;
		}
		return mix(r, w3, (dist + pixel_size / 2.0) / pixel_size);
	}
	if (_matchMask(pattern, 191, 143) || _matchMask(pattern, 126, 14))
	{
		float dist = p.x + 2.0 * p.y;
		float pixel_size = length(1.0 / (output_resolution / input_resolution)) * sqrt(5.0);

		if (dist > 1.0 + pixel_size / 2.0)
		{
			return w4;
		}

		vec4 r;
		if (is_different(w0, w1) || is_different(w0, w3))
		{
			r = mix(w1, w3, p.y - p.x + 0.5);
		}
		else
		{
			r = mix(mix(w1 * 0.375 + w0 * 0.25 + w3 * 0.375, w3, p.y * 2.0), w1, p.x * 2.0);
		}

		if (dist < 1.0 - pixel_size / 2.0)
		{
			return r;
		}

		return mix(r, w4, (dist + pixel_size / 2.0 - 1.0) / pixel_size);
	}

	if (_matchMask(pattern, 126, 42) || _matchMask(pattern, 239, 171))
	{
		float dist = p.y + 2.0 * p.x;
		float pixel_size = length(1.0 / (output_resolution / input_resolution)) * sqrt(5.0);

		if (dist > 1.0 + pixel_size / 2.0)
		{
			return w4;
		}

		vec4 r;

		if (is_different(w0, w1) || is_different(w0, w3))
		{
			r = mix(w1, w3, p.y - p.x + 0.5);
		}
		else
		{
			r = mix(mix(w1 * 0.375 + w0 * 0.25 + w3 * 0.375, w3, p.y * 2.0), w1, p.x * 2.0);
		}

		if (dist < 1.0 - pixel_size / 2.0)
		{
			return r;
		}

		return mix(r, w4, (dist + pixel_size / 2.0 - 1.0) / pixel_size);
	}

	if (_matchMask(pattern, 27, 3) || _matchMask(pattern, 79, 67) || _matchMask(pattern, 139, 131) || _matchMask(pattern, 107, 67))
	{
		return mix(w4, w3, 0.5 - p.x);
	}

	if (_matchMask(pattern, 75, 9) || _matchMask(pattern, 139, 137) || _matchMask(pattern, 31, 25) || _matchMask(pattern, 59, 25))
	{
		return mix(w4, w1, 0.5 - p.y);
	}

	if (_matchMask(pattern, 251, 106) || _matchMask(pattern, 111, 110) || _matchMask(pattern, 63, 62) || _matchMask(pattern, 251, 250) ||
		_matchMask(pattern, 223, 222) || _matchMask(pattern, 223, 30))
	{
		return mix(w4, w0, (1.0 - p.x - p.y) / 2.0);
	}

	if (_matchMask(pattern, 79, 75) || _matchMask(pattern, 159, 27) || _matchMask(pattern, 47, 11) ||
		_matchMask(pattern, 190, 10) || _matchMask(pattern, 238, 10) || _matchMask(pattern, 126, 10) || _matchMask(pattern, 235, 75) ||
		_matchMask(pattern, 59, 27))
	{
		float dist = p.x + p.y;
		float pixel_size = length(1.0 / (output_resolution / input_resolution));

		if (dist > 0.5 + pixel_size / 2.0)
		{
			return w4;
		}

		vec4 r;
		if (is_different(w0, w1) || is_different(w0, w3))
		{
			r = mix(w1, w3, p.y - p.x + 0.5);
		}
		else
		{
			r = mix(mix(w1 * 0.375 + w0 * 0.25 + w3 * 0.375, w3, p.y * 2.0), w1, p.x * 2.0);
		}

		if (dist < 0.5 - pixel_size / 2.0)
		{
			return r;
		}

		return mix(r, w4, (dist + pixel_size / 2.0 - 0.5) / pixel_size);
	}

	if (_matchMask(pattern, 11, 1))
	{
		return mix(mix(w4, w3, 0.5 - p.x), mix(w1, (w1 + w3) / 2.0, 0.5 - p.x), 0.5 - p.y);
	}

	if (_matchMask(pattern, 11, 0))
	{
		return mix(mix(w4, w3, 0.5 - p.x), mix(w1, w0, 0.5 - p.x), 0.5 - p.y);
	}

	float dist = p.x + p.y;
	float pixel_size = length(1.0 / (output_resolution / input_resolution));

	if (dist > 0.5 + pixel_size / 2.0)
	{
		return w4;
	}

	/* We need more samples to "solve" this diagonal */
	vec4 x0 = texture2D(image, position + vec2(-o.x * 2.0, -o.y * 2.0));
	vec4 x1 = texture2D(image, position + vec2(-o.x       , -o.y * 2.0));
	vec4 x2 = texture2D(image, position + vec2( 0.0       , -o.y * 2.0));
	vec4 x3 = texture2D(image, position + vec2( o.x       , -o.y * 2.0));
	vec4 x4 = texture2D(image, position + vec2(-o.x * 2.0, -o.y       ));
	vec4 x5 = texture2D(image, position + vec2(-o.x * 2.0,  0.0       ));
	vec4 x6 = texture2D(image, position + vec2(-o.x * 2.0,  o.y       ));

	if (is_different(x0, w4)) pattern += exp2(8.0);
	if (is_different(x1, w4)) pattern += exp2(9.0);
	if (is_different(x2, w4)) pattern += exp2(10.0);
	if (is_different(x3, w4)) pattern += exp2(11.0);
	if (is_different(x4, w4)) pattern += exp2(12.0);
	if (is_different(x5, w4)) pattern += exp2(13.0);
	if (is_different(x6, w4)) pattern += exp2(14.0);

	float diagonal_bias = -7.0;
	for (int i = 0; i < 15; ++i)
	{
		diagonal_bias += _bit(pattern, i);
	}

	if (diagonal_bias <= 0.0)
	{
		vec4 r = mix(w1, w3, p.y - p.x + 0.5);
		if (dist < 0.5 - pixel_size / 2.0)
		{
			return r;
		}
		return mix(r, w4, (dist + pixel_size / 2.0 - 0.5) / pixel_size);
	}

	return w4;
}

void main()
{
	gl_FragColor = scale(tex, texCoord, texSize, outputSize);
}
