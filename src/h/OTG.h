#ifndef OTES_H
#define OTES_H

#include "../../../OSAL/src/h/util.h"
#include "../../../OSAL/src/h/graphics.h"

/*@todo
add getters/setters for all entity members
create text_block_renderer (all it has to do is clip)
test out the rest of updating (relsize,relpos,angle,etc.)
use dirty so its not super inefficient, sleep
change childrens position based on renderangle of parent
*/

typedef struct renderer renderer;/*@interface*/

typedef struct entity entity;

entity *ctor_entity(entity *parent);
/*doesnt free renderers, you have to manage that separately (keep reference)*/
void dtor_entity(entity *e);

void entity_add_renderer(entity *e, renderer *r);
void entity_remove_renderer(entity *e, renderer *r);

void entity_set_order(entity *e, u32 order);
void entity_get_order(entity *e);
void entity_set_visible(entity *e,bool visible);
void entity_set_size(entity *e, vec2 size);
vec2 entity_get_size(entity *e);
void entity_set_position(entity *e, vec2 position);
vec2 entity_get_position(entity *e);
void entity_set_relsize(entity *e, vec2 relsize);
vec2 entity_get_relsize(entity *e);
void entity_set_relpos(entity *e, vec2 relpos);
vec2 entity_get_relpos(entity *e);
void entity_set_angle(entity *e, f32 angle);
f32 entity_get_angle(entity *e);

void update_entity_recursive(entity *e);
void render_entity_recursive(entity *e);  
//entity *hit_test_recursive(entity *e);

typedef struct text_block_renderer text_block_renderer;

void text_block_renderer_render(entity *e, text_block_renderer *t);
text_block_renderer *ctor_text_block_renderer(window *w, ttf_font *font, bool do_clip);
void text_block_renderer_set_text(text_block_renderer *t, char **text, u32 lines, color text_color, u32 *line_to_rerender/*NULL to rerender all lines*/);
/*doesnt free the font, you have to manage that separately (keep a reference)*/
void dtor_text_block_renderer(text_block_renderer *t);

/*typedef struct text_stretch_renderer text_stretch_renderer;

void text_stretch_renderer_render(entity *e, text_stretch_renderer *t);
text_stretch_renderer *ctor_text_stretch_renderer();
void dtor_text_stretch_renderer(text_stretch_renderer *t);
*/
typedef struct image_renderer image_renderer;

void image_renderer_render(entity *e, image_renderer *i);
image_renderer *ctor_image_renderer(window *w, texture *t);
void dtor_image_renderer(image_renderer *i);

#endif