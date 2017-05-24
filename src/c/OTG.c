#include "../../../OSAL/src/h/graphics.h"
#include "../../../OSAL/src/h/input.h"
#include "../../../OSAL/src/h/util.h"
#include "../../../OSAL/src/h/system.h"
#include "../h/OTG.h"

#include <stdlib.h>
#include <stdio.h>
/*@todo

@notes

can't currently change a renderer to render from one window to another, it creates the texture for the sdl_renderer specific to the window
*/

typedef struct renderer
{
    bool dirty;//this necessary? ...
    void (*render)(entity *e, renderer *r);
} renderer;

u32 uid_counter=0;
typedef struct entity
{
	vec2 render_size;
	vec2 render_position;

	entity *parent;
    u32 uid;

    entity **children;
    u32 children_size;

    renderer **renderers;
    u32 renderers_size;

    u32 order;

	bool dirty;
    bool visible;
	
	vec2 size;
	vec2 position;
	
	vec2 relsize;
	vec2 relpos;

	f32 angle;
    f32 render_angle;
} entity;
/*
@todo allocate renderers and entity in one block instead of array of pointers?
*/
entity *ctor_entity(entity *parent)
{
    entity *e=(entity*)malloc(sizeof(entity));

    e->parent=parent;

    if(e->parent)
    {
        e->parent->children_size+=1;
        e->parent->children=realloc(e->parent->children, sizeof(entity *) * (e->parent->children_size));
        e->parent->children[e->parent->children_size-1]=e;
        e->order=e->parent->children_size-1;
        entity_set_order(e,e->order);
        //push myself to parent array
    }

    e->children=NULL;
    e->children_size=0;

    e->renderers=NULL;
    e->renderers_size=0;

    e->visible=true;
    e->dirty=true;

    e->render_size.x=0;
    e->render_size.y=0;

    e->render_position.x=0;
    e->render_position.y=0;

    e->uid=uid_counter+=1;

    e->size.x=0;
    e->size.y=0;
    
    e->position.x=0;
    e->position.y=0;

    e->relsize.x=0;
    e->relsize.y=0;

    e->relpos.x=0;
    e->relpos.y=0;

    e->angle=0;
    e->render_angle=0;

    return e;
}
/*doesnt free renderers, you have to manage that separately (keep reference)*/
void dtor_entity(entity *e)
{
    u32 i;

    if(e->parent)
    {
        for(i=0; i<e->parent->children_size; i++)
        {
            if(e->parent->children[i]->uid==e->uid)
            {
                for(u32 i2=i; i2<e->parent->children_size-1; i2++)
                {
                    e->parent->children[i2]=e->parent->children[i2+1];
                }
                e->parent->children_size-=1;
                e->parent->children=realloc(e->parent->children,e->parent->children_size*sizeof(entity*));
            }
        }
    }
    for(i=0; i<e->children_size; i+=1)
    {
        dtor_entity(e->children[i]);
    }
    
    free(e);
}
void entity_set_order(entity *e, u32 order)
{    
    e->order=order;
    entity *temp;
    for(u32 i=0; i<e->parent->children_size-1; i++)
    {
        for(u32 i2=0; i2<e->parent->children_size - i - 1; i2++)
        {
            if(e->parent->children[i2]->order > e->parent->children[i2+1]->order)
            {
                temp=e->parent->children[i2];
                e->parent->children[i2]=e->parent->children[i2+1];
                e->parent->children[i2+1]=temp;
            }
        }
    }
}
u32 entity_get_order(entity *e)
{
    return e->order;
}
void entity_add_renderer(entity *e, renderer *r)
{
    e->renderers=realloc(e->renderers, sizeof(renderer *) * (e->renderers_size+=1));
    e->renderers[e->renderers_size-1]=r;
}
void entity_remove_renderer(entity *e, renderer *r)
{
    u32 i;
    u32 found=false;
    for(i=0; i< e->renderers_size; i+=1)
    {
        if(e->renderers[i]==r)
        {
            found=true;
            break;
        }   
    }

    if(found)
    {
        u32 i2;
        for(i2=i; i2<e->renderers_size-1; i2+=1)
        {
            e->renderers[i2]=e->renderers[i2+1];
        }
        e->renderers_size-=1;
    }
}
void entity_set_visible(entity *e,bool visible)
{
    e->visible=visible;
}
void entity_set_size(entity *e, vec2 size)
{
    e->size=size;
}
vec2 entity_get_size(entity *e)
{
    return e->size;
}
void entity_set_position(entity *e, vec2 position)
{
    e->position=position;
}
vec2 entity_get_position(entity *e)
{
    return e->position;
}
void entity_set_relsize(entity *e, vec2 relsize)
{
    e->relsize=relsize;
}
vec2 entity_get_relsize(entity *e)
{
    return e->relsize;
}
void entity_set_relpos(entity *e, vec2 relpos)
{
    e->relpos=relpos;
}
vec2 entity_get_relpos(entity *e)
{
    return e->relpos;
}
void entity_set_angle(entity *e, f32 angle)
{
    e->angle=angle;
}
f32 entity_get_angle(entity *e)
{
    return e->angle;
}
vec2 entity_get_render_position(entity *e)
{
    return e->render_position;
}
vec2 entity_get_render_size(entity *e)
{
    return e->render_size;
}

void update_entity_recursive(entity *e)
{
    u32 i;

    e->render_position.x = e->position.x + (e->parent ? e->parent->render_position.x + e->parent->render_size.x * e->relpos.x : 0);
    e->render_position.y = e->position.y + (e->parent ? e->parent->render_position.y + e->parent->render_size.y * e->relpos.y: 0);

    e->render_size.x = e->size.x + (e->parent ? e->relsize.x * e->parent->render_size.x : 0);
    e->render_size.y = e->size.y + (e->parent ? e->relsize.y * e->parent->render_size.y : 0);

    e->render_angle = e->angle + (e->parent ? e->parent->render_angle : 0);

    for(i=0; i<e->children_size; i+=1)
    {
        update_entity_recursive(e->children[i]);
    }
}
void render_entity_recursive(entity *e)
{
    u32 i;

    if(!e->visible)return;

    for(i=0; i<e->renderers_size; i+=1)
    {
        e->renderers[i]->render(e,e->renderers[i]);
    }

    for(i=0; i<e->children_size; i+=1)
    {
        render_entity_recursive(e->children[i]);
    }
}
entity *hit_test_recursive(entity *e)
{

}

typedef struct text_block_renderer
{
    renderer r;
    ttf_font *f;
    /*@todo handle deleting lines? rerender all option on set_text?*/
    texture **font_textures;/*texture for each line*/
    texture **line_number_textures;/*texture for each line*/
    char **text;
    bool do_clip;
    u32 *line_numbers;
    u32 lines;
    window *w;
} text_block_renderer;

#define MAXIMUM_LINE_NUMBER_LENGTH 20/*@bug only supports length 20 line numbers*/
void text_block_renderer_render(entity *e, text_block_renderer *tbr)
{
    /*@todo
    syntax highlighting (pass grammar file path as parameter?) char const *, color union in set text?
    */

    /*
    will not render outside of rendering area (window_get_size(w))

    entity region is clip area
    */
    u32 i;
    for(i=0; i<tbr->lines; i++)
    {
        if(tbr->line_numbers)
        {
            if(tbr->line_number_textures[i] && tbr->font_textures[i])
            {
                rect r;
                rect r2;

                rect clip_region;
                
                int size_width;
                int size_height;
                
                char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
                itoa(i,buffer,10);

                size_ttf_font(tbr->f,buffer,&size_width,&size_height);
                r2.w=size_width;
                r2.h=size_height;
                r2.x=e->render_position.x;
                r2.y=e->render_position.y+size_height*i;

                r.x=e->render_position.x+*tbr->line_numbers;
                r.y=e->render_position.y+size_height*i;

                size_ttf_font(tbr->f,tbr->text[i],&size_width,&size_height);
                r.w=size_width;
                r.h=size_height;
                
                clip_region.x=e->render_position.x;
                clip_region.y=e->render_position.y;
                clip_region.w=e->render_size.x;
                clip_region.h=e->render_size.y;

                draw_texture(tbr->w, tbr->line_number_textures[i], &r2, e->render_angle, NULL, NULL, tbr->do_clip ? &clip_region : 0);
                draw_texture(tbr->w, tbr->font_textures[i], &r, e->render_angle, NULL, NULL, tbr->do_clip ? &clip_region : 0);
            }
        }
        else
        {
            if(tbr->font_textures[i])
            {
                rect r;
                rect clip_region;
                
                int size_width;
                int size_height;
                size_ttf_font(tbr->f,tbr->text[i],&size_width,&size_height);
                r.w=size_width;
                r.h=size_height;
                
                r.x=e->render_position.x;
                r.y=e->render_position.y+size_height*i;

                clip_region.x=e->render_position.x;
                clip_region.y=e->render_position.y;
                clip_region.w=e->render_size.x;
                clip_region.h=e->render_size.y;

                
                draw_texture(tbr->w, tbr->font_textures[i], &r, e->render_angle, NULL, NULL, tbr->do_clip ? &clip_region : 0);
            }
        }
    }
}
text_block_renderer *ctor_text_block_renderer(window *w, ttf_font *font, bool do_clip, u32 *line_numbers)
{
    text_block_renderer *tbr=(text_block_renderer*)malloc(sizeof(text_block_renderer));
    tbr->r.render=text_block_renderer_render;
    tbr->line_numbers=line_numbers;
    tbr->do_clip=do_clip;
    tbr->f=font;
    tbr->w=w;
    tbr->font_textures=NULL;
    tbr->line_number_textures=NULL;
    tbr->text=NULL;
    tbr->lines=0;

    return tbr;
}
void text_block_renderer_set_text(text_block_renderer *t, char **text, u32 lines, color text_color, u32 *line_to_rerender)
{
    u32 i;

    /*if the array changes length we rerender (recreate) everything, otherwise its on you to tell when to recreate everything*/
    /*@perf instead of rerendering everything if lines != t->lines just rerender latest one? and count on them to rerender everything if they inserted or something?*/
    if(!line_to_rerender || lines != t->lines)
    {
        if(!t->font_textures)
        {
            t->font_textures=(texture**)calloc(lines,sizeof(texture*));
            if(t->line_numbers)
            {
                t->line_number_textures=(texture**)calloc(lines,sizeof(texture*));
            }
        }
        else
        {
            for(i=0; i<t->lines; i++)
            {
                dtor_texture(t->font_textures[i]);
            }
            free(t->font_textures);
            t->font_textures=(texture**)calloc(lines,sizeof(texture*));

            if(t->line_numbers)
            {
                for(i=0; i<t->lines; i++)
                {
                    dtor_texture(t->line_number_textures[i]);
                }
                free(t->line_number_textures);
                t->line_number_textures=(texture**)calloc(lines,sizeof(texture*));
            }
        }

        for(i=0; i<lines; i++)
        {
            if(t->font_textures[i])
            {
                dtor_texture(t->font_textures[i]);
            }
            t->font_textures[i]=ctor_texture_font(t->w,t->f,text[i],text_color);
        
            if(t->line_numbers)
            {
                if(t->line_number_textures[i])
                {
                    dtor_texture(t->line_number_textures[i]);
                }
                char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
                itoa(i,buffer,10);
                t->line_number_textures[i]=ctor_texture_font(t->w,t->f,buffer,text_color);
                texture_set_alpha(t->line_number_textures[i],100);
            }
        }
    }
    else
    {
        dtor_texture(t->font_textures[*line_to_rerender]);
        t->font_textures[*line_to_rerender]=ctor_texture_font(t->w,t->f,text[*line_to_rerender],text_color);

        if(t->line_numbers)
        {
            dtor_texture(t->line_number_textures[*line_to_rerender]);
            char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
            itoa(*line_to_rerender,buffer,10);
            t->line_number_textures[*line_to_rerender]=ctor_texture_font(t->w,t->f,buffer,text_color);
            texture_set_alpha(t->line_number_textures[*line_to_rerender],100);
        }
    }

    t->text=text;
    t->lines=lines;
}
/*doesnt free the font, you have to manage that separately (keep a reference)*/
void dtor_text_block_renderer(text_block_renderer *t)
{
    u32 i;
    
    if(t->font_textures)
    {
        for(i=0; i<t->lines; i+=1)
        {
            dtor_texture(t->font_textures[i]);
        }
        
        free(t->font_textures);
    }
    if(t->line_number_textures)
    {
        for(i=0; i<t->lines; i+=1)
        {
            dtor_texture(t->line_number_textures[i]);
        }
        free(t->line_number_textures);
    }
    free(t);
}

/*
typedef struct text_stretch_renderer
{
    renderer r;
    char const *text;
} text_stretch_renderer;
void text_stretch_renderer_render(entity *e, text_stretch_renderer *t)
{

}
text_stretch_renderer *ctor_text_stretch_renderer()
{

}
void dtor_text_stretch_renderer(text_stretch_renderer *t)
{
    
}
*/

typedef struct image_renderer
{
    renderer r;
    texture *image_texture;
    window *w;
} image_renderer;
void image_renderer_render(entity *e, image_renderer *i)
{
    rect r;
    r.x=e->render_position.x;
    r.y=e->render_position.y;
    r.w=e->render_size.x;
    r.h=e->render_size.y;

    draw_texture(i->w, i->image_texture, &r, e->render_angle, NULL, NULL, NULL);
}
image_renderer *ctor_image_renderer(window *w,texture *t)
{
    image_renderer *img=(image_renderer*)malloc(sizeof(image_renderer));
    img->w=w;
    img->image_texture=t;
    img->r.render=image_renderer_render;
    return img;
}
void dtor_image_renderer(image_renderer *img)
{
    dtor_texture(img->image_texture);
    free(img);
}