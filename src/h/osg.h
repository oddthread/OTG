/*This file was automatically generated.*/
#ifndef d_osg_h
#define d_osg_h

#include "oul/src/h/oul.h"
#include "opl/src/h/graphics.h"

#include "ovp/src/h/ovp.h"

typedef struct entity entity;
typedef struct renderer renderer;
typedef struct text_block_renderer text_block_renderer;
typedef struct text_stretch_renderer text_stretch_renderer;
typedef struct image_renderer image_renderer;

void dtor_image_renderer(image_renderer *img);
void image_renderer_set_texture(image_renderer *img,texture *t);
image_renderer *ctor_image_renderer(window *w,texture *t,rect *clip_region);
void image_renderer_render(entity *e,renderer *i_r);
void dtor_text_stretch_renderer(text_stretch_renderer *t);
text_stretch_renderer *ctor_text_stretch_renderer();
void text_stretch_renderer_render(entity *e,text_stretch_renderer *t);
void dtor_text_block_renderer(text_block_renderer *t);
void text_block_renderer_set_text(text_block_renderer *t,char **text,u32 lines,color text_color,u32 *line_to_rerender,char *ext);
text_block_renderer *ctor_text_block_renderer(window *w,ttf_font *font,bool do_clip,u32 *line_numbers,char *alignment, ovp *config);
void text_block_renderer_render(entity *e,renderer *tbr_r);
entity *hit_test_recursive(vec2 mouse_position,entity *e,entity *highest);
bool entity_is_or_is_recursive_child(entity *e,entity *test);
void render_entity_recursive(entity *e);
void update_entity_recursive(entity *e);
r32 entity_get_alpha(entity *e);
void entity_set_alpha(entity *e,r32 alpha);
void entity_set_solid(entity *e,bool solid);
bool entity_get_solid(entity *e);
vec2 entity_get_render_size(entity *e);
vec2 entity_get_render_position(entity *e);
r32 entity_get_angle(entity *e);
void entity_set_angle(entity *e,r32 angle);
vec2 entity_get_relposme(entity *e);
void entity_set_relposme(entity *e,vec2 relposme);
vec2 entity_get_relpos(entity *e);
void entity_set_relpos(entity *e,vec2 relpos);
vec2 entity_get_relsize(entity *e);
void entity_set_relsize(entity *e,vec2 relsize);
vec2 entity_get_position(entity *e);
void entity_set_position(entity *e,vec2 position);
vec2 entity_get_size(entity *e);
void entity_set_size(entity *e,vec2 size);
void entity_set_visible(entity *e,bool visible);
void entity_remove_renderer(entity *e,renderer *r);
void entity_add_renderer(entity *e,renderer *r);
u32 entity_get_order(entity *e);
void entity_sort_children(entity *e);
void dtor_entity(entity *e);
void entity_set_order(entity *e,u32 order);
entity *ctor_entity(entity *parent);
extern u32 uid_counter;
#define offset_margin 10
#define MAXIMUM_LINE_NUMBER_LENGTH 200

#endif
