#ifndef SYNTAX_HIGHLIGHT_H
#define SYNTAX_HIGHLIGHT_H

#include "opl/src/h/graphics.h"
#include "ovp/src/h/ovp.h"

typedef struct line_textures{
    texture **textures;/*in order*/
    rect *textures_rects;
    int textures_sz;
} line_textures;

void init_line_textures(line_textures *lt, window *w, char *line, ovp *config, ttf_font *f, char *ext);
void release_line_textures(line_textures *lt);

#endif