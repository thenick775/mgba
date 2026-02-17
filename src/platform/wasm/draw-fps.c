#include "draw-fps.h"

typedef struct {
	uint8_t x, y, w, h;
} GlyphRect;

typedef struct {
	char c;
	const GlyphRect* rects;
	uint8_t n;
} Glyph;

static void _fillRectRGBA(uint8_t* rgba, int bufW, int bufH, int x, int y, int w, int h, uint8_t r, uint8_t g,
                          uint8_t b, uint8_t a) {
	if (w <= 0 || h <= 0)
		return;

	// Clip
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 > bufW)
		x1 = bufW;
	if (y1 > bufH)
		y1 = bufH;

	if (x0 >= x1 || y0 >= y1)
		return;

	for (int yy = y0; yy < y1; ++yy) {
		size_t row = ((size_t) yy * (size_t) bufW + (size_t) x0) * 4u;
		for (int xx = x0; xx < x1; ++xx) {
			rgba[row + 0] = r;
			rgba[row + 1] = g;
			rgba[row + 2] = b;
			rgba[row + 3] = a;
			row += 4u;
		}
	}
}

static const GlyphRect GLYPH_0[] = {
	{ 0, 3, 2, 5 }, { 1, 2, 2, 1 }, { 1, 8, 2, 1 }, { 2, 1, 3, 1 }, { 2, 9, 3, 1 },
	{ 3, 5, 1, 1 }, { 4, 2, 2, 1 }, { 4, 8, 2, 1 }, { 5, 3, 2, 5 },
};
static const GlyphRect GLYPH_1[] = {
	{ 1, 3, 4, 1 }, { 1, 9, 6, 1 }, { 2, 2, 3, 1 }, { 3, 1, 2, 1 }, { 3, 4, 2, 5 },
};
static const GlyphRect GLYPH_2[] = {
	{ 0, 2, 2, 1 }, { 0, 8, 2, 2 }, { 1, 1, 5, 1 }, { 1, 7, 2, 1 }, { 2, 6, 2, 1 },
	{ 2, 9, 5, 1 }, { 3, 5, 2, 1 }, { 4, 4, 2, 1 }, { 5, 2, 2, 2 }, { 5, 8, 2, 1 },
};
static const GlyphRect GLYPH_3[] = {
	{ 0, 2, 2, 1 }, { 0, 8, 2, 1 }, { 1, 1, 5, 1 }, { 1, 9, 5, 1 }, { 2, 5, 4, 1 }, { 5, 2, 2, 3 }, { 5, 6, 2, 3 },
};
static const GlyphRect GLYPH_4[] = {
	{ 0, 5, 2, 2 }, { 1, 4, 2, 1 }, { 2, 3, 4, 1 }, { 2, 6, 5, 1 }, { 3, 2, 3, 1 },
	{ 3, 9, 4, 1 }, { 4, 1, 2, 1 }, { 4, 4, 2, 2 }, { 4, 7, 2, 2 },
};
static const GlyphRect GLYPH_5[] = {
	{ 0, 1, 7, 1 }, { 0, 2, 2, 4 }, { 0, 8, 2, 1 }, { 1, 9, 5, 1 }, { 2, 5, 4, 1 }, { 5, 6, 2, 3 },
};
static const GlyphRect GLYPH_6[] = {
	{ 0, 3, 2, 6 }, { 1, 2, 2, 1 }, { 1, 9, 5, 1 }, { 2, 1, 3, 1 }, { 2, 5, 4, 1 }, { 5, 6, 2, 3 },
};
static const GlyphRect GLYPH_7[] = {
	{ 0, 1, 7, 1 }, { 0, 2, 2, 1 }, { 2, 6, 2, 4 }, { 3, 5, 2, 1 }, { 4, 4, 2, 1 }, { 5, 2, 2, 2 },
};
static const GlyphRect GLYPH_8[] = {
	{ 0, 2, 2, 3 }, { 0, 6, 2, 3 }, { 1, 1, 5, 1 }, { 1, 5, 5, 1 }, { 1, 9, 5, 1 }, { 5, 2, 2, 3 }, { 5, 6, 2, 3 },
};
static const GlyphRect GLYPH_9[] = {
	{ 0, 2, 2, 3 }, { 1, 1, 5, 1 }, { 1, 5, 6, 1 }, { 1, 9, 4, 1 }, { 4, 8, 2, 1 }, { 5, 2, 2, 3 }, { 5, 6, 2, 2 },
};
static const GlyphRect GLYPH_DOT[] = { { 3, 8, 2, 2 } };

static const Glyph GLYPHS[] = {
	{ '0', GLYPH_0, (uint8_t) (sizeof(GLYPH_0) / sizeof(GLYPH_0[0])) },
	{ '1', GLYPH_1, (uint8_t) (sizeof(GLYPH_1) / sizeof(GLYPH_1[0])) },
	{ '2', GLYPH_2, (uint8_t) (sizeof(GLYPH_2) / sizeof(GLYPH_2[0])) },
	{ '3', GLYPH_3, (uint8_t) (sizeof(GLYPH_3) / sizeof(GLYPH_3[0])) },
	{ '4', GLYPH_4, (uint8_t) (sizeof(GLYPH_4) / sizeof(GLYPH_4[0])) },
	{ '5', GLYPH_5, (uint8_t) (sizeof(GLYPH_5) / sizeof(GLYPH_5[0])) },
	{ '6', GLYPH_6, (uint8_t) (sizeof(GLYPH_6) / sizeof(GLYPH_6[0])) },
	{ '7', GLYPH_7, (uint8_t) (sizeof(GLYPH_7) / sizeof(GLYPH_7[0])) },
	{ '8', GLYPH_8, (uint8_t) (sizeof(GLYPH_8) / sizeof(GLYPH_8[0])) },
	{ '9', GLYPH_9, (uint8_t) (sizeof(GLYPH_9) / sizeof(GLYPH_9[0])) },
	{ '.', GLYPH_DOT, (uint8_t) (sizeof(GLYPH_DOT) / sizeof(GLYPH_DOT[0])) },
};

static const Glyph* _glyphFor(char c) {
	for (size_t i = 0; i < sizeof(GLYPHS) / sizeof(GLYPHS[0]); ++i) {
		if (GLYPHS[i].c == c)
			return &GLYPHS[i];
	}
	return NULL;
}

static void _drawCharRGBA(uint8_t* rgba, int bufW, int bufH, char c, int scale, int x, int y, uint8_t r, uint8_t g,
                          uint8_t b, uint8_t a) {
	const Glyph* glyph = _glyphFor(c);
	if (!glyph)
		return;

	for (uint8_t i = 0; i < glyph->n; ++i) {
		const GlyphRect gr = glyph->rects[i];
		_fillRectRGBA(rgba, bufW, bufH, x + (int) gr.x * scale, y + (int) gr.y * scale, (int) gr.w * scale,
		              (int) gr.h * scale, r, g, b, a);
	}
}

static void _drawTextRGBA(uint8_t* rgba, int bufW, int bufH, const char* text, int scale, int x, int y, uint8_t r,
                          uint8_t g, uint8_t b, uint8_t a) {
	for (int i = 0; text[i] != '\0'; ++i) {
		_drawCharRGBA(rgba, bufW, bufH, text[i], scale, x + (8 * scale * i), y, r, g, b, a);
	}
}

// Draws a gray background + white text into outputBuffer
void drawFPSOverlayIntoOutputBuffer(unsigned x, unsigned y, unsigned bufW, unsigned bufH, uint8_t* outputBuffer,
                                    double fps) {
	if (!outputBuffer || bufW == 0 || bufH == 0)
		return;

	char fpsBuf[32];
	snprintf(fpsBuf, sizeof(fpsBuf), "%.1f", fps);

	const int scale = 1;
	const int charW = 8 * scale;
	const int charH = 10 * scale;
	const int textW = (int) strlen(fpsBuf) * charW;
	const int textH = charH;

	uint8_t* rgba = outputBuffer;

	_fillRectRGBA(rgba, (int) bufW, (int) bufH, (int) x - 2, (int) y - 2, textW + 4, textH + 4, 64, 64, 64, 255);

	_drawTextRGBA(rgba, (int) bufW, (int) bufH, fpsBuf, scale, (int) x, (int) y, 255, 255, 255, 255);
}