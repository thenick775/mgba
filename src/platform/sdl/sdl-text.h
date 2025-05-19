#ifndef SDL_TEXT_H_INCLUDED
#define SDL_TEXT_H_INCLUDED

#include <SDL.h>
#include <ctype.h>

#define LINE_HEIGHT 12

// Lazy text rendering for SDL without fonts or any other dependencies using only RenderFillRect
// sourced from https://gist.github.com/Introscopia/e74413c0950b699fa3371aaba40ca003 and modified

void SDL_RenderChar(SDL_Renderer* R, char C, float scale, float x, float y);
void SDL_RenderText(SDL_Renderer* R, char* string, float scale, float x, float y);

#endif