#ifndef SDL_TEXT_H_INCLUDED
#define SDL_TEXT_H_INCLUDED

#include <SDL.h>
#include <ctype.h>

#define LINE_HEIGHT 12

void SDL_RenderChar( SDL_Renderer *R, char C, float scale, float x, float y );
void SDL_RenderText( SDL_Renderer *R, char *string, float scale, float x, float y );
void SDL_RenderTextWrapped( SDL_Renderer *R, char *string, float scale, float x, float y, float width );

#endif