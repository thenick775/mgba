#include "sdl-text.h"

void SDL_RenderChar(SDL_Renderer* R, char C, float scale, float x, float y) {
	switch (C) {
	case '!':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 4 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '"':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (0 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (0 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		break;
	case '#':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '$':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (0 * scale) + y, 1 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 1 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '%':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		break;
	case '&':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '\'':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (0 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '(':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case ')':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		break;
	case '*':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (4 * scale) + y, 4 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (7 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		break;
	case '+':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (3 * scale) + y, 2 * scale, 6 * scale });
		break;
	case ',':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (10 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '-':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 7 * scale, 1 * scale });
		break;
	case '.':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '/':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (1 * scale) + y, 1 * scale, 1 * scale });
		break;
	case '0':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		break;
	case '1':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 2 * scale, 5 * scale });
		break;
	case '2':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '3':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '4':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (3 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '5':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '6':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '7':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '8':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case '9':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 2 * scale });
		break;
	case ':':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		break;
	case ';':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '<':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '=':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (7 * scale) + y, 6 * scale, 1 * scale });
		break;
	case '>':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '?':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		break;
	case '@':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 4 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 'A':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'B':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'C':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (7 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'D':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		break;
	case 'E':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (7 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'F':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'G':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (9 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'H':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 9 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'I':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		break;
	case 'J':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		break;
	case 'K':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 'L':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (7 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'M':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 9 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (3 * scale) + y, 4 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 5 * scale });
		break;
	case 'N':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 9 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (3 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 3 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'O':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 5 * scale });
		break;
	case 'P':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'Q':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 1 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (10 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'R':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'S':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 'T':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 6 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (3 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'U':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 8 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 8 * scale });
		break;
	case 'V':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (8 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 6 * scale });
		break;
	case 'W':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (7 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 1 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 6 * scale });
		break;
	case 'X':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (4 * scale) + y, 3 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 'Y':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'Z':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (7 * scale) + y, 1 * scale, 1 * scale });
		break;
	case '[':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 2 * scale, 8 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '\\':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (3 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (6 * scale) + x, (7 * scale) + y, 1 * scale, 3 * scale });
		break;
	case ']':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		break;
	case '^':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (0 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '_':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (11 * scale) + y, 7 * scale, 1 * scale });
		break;
	case '`':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (0 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'a':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'b':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 8 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'c':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'd':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (4 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'e':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'f':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (3 * scale) + y, 1 * scale, 1 * scale });
		break;
	case 'g':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (10 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (11 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'h':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 5 * scale });
		break;
	case 'i':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'j':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (11 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 6 * scale });
		break;
	case 'k':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 'l':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 7 * scale });
		break;
	case 'm':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 3 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (6 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (5 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 1 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (6 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'n':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 5 * scale });
		break;
	case 'o':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'p':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (11 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 2 * scale, 6 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (8 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 3 * scale });
		break;
	case 'q':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (11 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'r':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (4 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 2 * scale });
		break;
	case 's':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (6 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (7 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 't':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (2 * scale) + y, 2 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (5 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 1 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'u':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (4 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'v':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (4 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (8 * scale) + y, 4 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'w':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 5 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (8 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 1 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 4 * scale });
		break;
	case 'x':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (6 * scale) + y, 3 * scale, 2 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'y':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (11 * scale) + y, 5 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 6 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (10 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (4 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (9 * scale) + y, 2 * scale, 1 * scale });
		break;
	case 'z':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (4 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (9 * scale) + y, 7 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (2 * scale) + x, (7 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (8 * scale) + y, 2 * scale, 1 * scale });
		break;
	case '{':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		break;
	case '|':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (1 * scale) + y, 2 * scale, 4 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 4 * scale });
		break;
	case '}':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (9 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (6 * scale) + y, 2 * scale, 3 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (4 * scale) + x, (5 * scale) + y, 3 * scale, 1 * scale });
		break;
	case '~':
		SDL_RenderFillRectF(R, &(SDL_FRect) { (0 * scale) + x, (2 * scale) + y, 2 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (1 * scale) + x, (1 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (3 * scale) + x, (2 * scale) + y, 3 * scale, 1 * scale });
		SDL_RenderFillRectF(R, &(SDL_FRect) { (5 * scale) + x, (1 * scale) + y, 2 * scale, 1 * scale });
		break;
	}
}

void SDL_RenderText(SDL_Renderer* R, char* string, float scale, float x, float y) {
	for (int i = 0; string[i] != '\0'; ++i) {
		SDL_RenderChar(R, string[i], scale, x + (8 * scale * i), y);
	}
}
