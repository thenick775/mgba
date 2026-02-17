#ifndef DRAW_FPS_H_INCLUDED
#define DRAW_FPS_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void drawFPSOverlayIntoOutputBuffer(unsigned x, unsigned y, unsigned bufW, unsigned bufH, uint8_t* outputBuffer,
                                    double fps);

#endif
