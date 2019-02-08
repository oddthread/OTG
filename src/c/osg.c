#include "osg/src/h/osg.h"

#include "opl/src/h/input.h"
#include "opl/src/h/system.h"

#include "oul/src/h/oul.h"
#include "osg/src/h/syntax_highlight.h"

/*@todo
add getters/setters for all entity members
create text_block_renderer (all it has to do is clip)
test out the rest of updating (relsize,relpos,angle,etc.)
use dirty so its not super inefficient, sleep
change childrens position based on renderangle of parent

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
    
    bool solid;
    
    entity **children;
    u32 children_size;

    renderer **renderers;
    u32 renderers_size;

    u32 order;

    r32 alpha;
    r32 render_alpha;

    bool dirty;
    bool visible;
	
	vec2 size;
	vec2 position;
	
	vec2 relsize;
	vec2 relpos;
	vec2 relposme;

	r32 angle;
    r32 render_angle;
} entity;
/*
@todo allocate renderers and entity in one block instead of array of pointers?
*/
entity *ctor_entity(entity *parent)
{
    entity *e=(entity*)mem_alloc(sizeof(entity));
    zero_out(e,sizeof(entity));

    e->parent=parent;

    if(e->parent)
    {
        e->parent->children_size+=1;
        e->parent->children=mem_realloc(e->parent->children, sizeof(entity *) * (e->parent->children_size));
        e->parent->children[e->parent->children_size-1]=e;
        e->order=e->parent->children_size-1;
        entity_set_order(e,e->order);
        //push myself to parent array
    }
    e->alpha=1;
    e->render_alpha=1;
    e->solid=true;
    e->visible=true;
    e->dirty=true;

    e->uid=uid_counter+=1;

    return e;
}
/*doesnt mem_free renderers, you have to manage that separately (keep reference)*/
void dtor_entity(entity *e)
{
    u32 i;

    if(e->parent)
    {
        for(i=0; i<e->parent->children_size; i++)
        {
            if(e->parent->children[i]->uid==e->uid)
            {
                d_remove(e->parent->children,e->parent->children_size,i,);
                break;
            }
        }
    }
    for(i=0; i<e->children_size; i+=1)
    {
        dtor_entity(e->children[i]);
    }
    
    mem_free(e);
}
void entity_set_order(entity *e, u32 order)
{    
    e->order=order;
}
void entity_sort_children(entity *e)
{
    entity *temp;
    for(u32 i=0; i<e->children_size-1; i++)
    {
        for(u32 i2=0; i2<e->children_size - i - 1; i2++)
        {
            if(e->children[i2]->order > e->children[i2+1]->order)
            {
                temp=e->children[i2];
                e->children[i2]=e->children[i2+1];
                e->children[i2+1]=temp;
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
    e->renderers=mem_realloc(e->renderers, sizeof(renderer *) * (e->renderers_size+=1));
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
void entity_set_relposme(entity *e, vec2 relposme)
{
    e->relposme=relposme;
}
vec2 entity_get_relposme(entity *e)
{
    return e->relposme;
}
void entity_set_angle(entity *e, r32 angle)
{
    e->angle=angle;
}
r32 entity_get_angle(entity *e)
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
bool entity_get_solid(entity *e)
{
    return e->solid;
}
void entity_set_solid(entity *e,bool solid)
{
    e->solid=solid;
}
void entity_set_alpha(entity *e, r32 alpha)
{
    e->alpha=alpha;
}
r32 entity_get_alpha(entity *e)
{
    return e->alpha;
}

void update_entity_recursive(entity *e)
{
    u32 i;
    
    e->render_position.x = e->position.x + (e->parent ? e->parent->render_position.x + e->parent->render_size.x * e->relpos.x : 0);
    e->render_position.y = e->position.y + (e->parent ? e->parent->render_position.y + e->parent->render_size.y * e->relpos.y: 0);

    e->render_size.x = e->size.x + (e->parent ? e->relsize.x * e->parent->render_size.x : 0);
    e->render_size.y = e->size.y + (e->parent ? e->relsize.y * e->parent->render_size.y : 0);
    
    e->render_position.x+=e->relposme.x*e->render_size.x;
    e->render_position.y+=e->relposme.y*e->render_size.y;
    
    e->render_angle = e->angle + (e->parent ? e->parent->render_angle : 0);
    
    e->render_alpha = e->alpha * (e->parent ? e->parent->render_alpha : 1);
    
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
bool entity_is_or_is_recursive_child(entity *e, entity *test)
{
    if(test==e)
    {
        return true;
    }
    for(u32 i=0; i<e->children_size; i++)
    {
        if(entity_is_or_is_recursive_child(e->children[i],test))
        {
            return true;
        }
    }
    return false;
}
entity *hit_test_recursive(vec2 mouse_position, entity *e, entity *highest)
{
    u32 i;

    if(!e->solid || !e->visible) return highest;
    
    if(insec(mouse_position,value_vec2(1,1),entity_get_render_position(e),entity_get_render_size(e)))
    {
    
        highest=e;
    }
    for(i=0; i<e->children_size; i+=1)
    {
        highest=hit_test_recursive(mouse_position, e->children[i], highest);
    }
    
    return highest;
}

typedef struct text_block_renderer
{
    renderer r;
    ttf_font *f;
    /*@todo handle deleting lines? rerender all option on set_text?*/
    texture **font_textures;/*texture for each line*/
    texture **line_number_textures;/*texture for each line*/
    char **text;
    char *alignment;
    bool do_clip;
    u32 *line_numbers;
    s32 lines;
    window *w;
} text_block_renderer;

void text_block_renderer_render(entity *e, renderer *tbr_r)
{
    text_block_renderer *tbr=(text_block_renderer*)tbr_r;
    /*@todo
    syntax highlighting (pass grammar file path as parameter?) char const *, color union in set text?
    */

    /*
    will not render outside of rendering area (window_get_size(w))

    entity region is clip area
    */
    u32 i;
    u32 text_offset_x=0;

    rect clip_region;                

    rect r;
    int size_width;
    int size_height;

    char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
    for(i=0; i<tbr->lines; i++)
    {
        if(*tbr->line_numbers)
        {
            text_offset_x=*tbr->line_numbers;
            
            itoa(i+1,buffer,10);
            size_ttf_font(tbr->f,buffer,&size_width,&size_height);
            r.w=size_width;
            r.h=size_height;
            r.x=/*e->render_position.x +*/ (text_offset_x-r.w-offset_margin);
            r.y=e->render_position.y+size_height*i;
            
            if(tbr->line_number_textures && tbr->line_number_textures[i])draw_texture(tbr->w, tbr->line_number_textures[i], &r, e->render_angle, NULL, NULL, NULL);
        }
        if(tbr->font_textures[i])
        {
            size_ttf_font(tbr->f,tbr->text[i],&size_width,&size_height);
            
            clip_region.x=text_offset_x;
            clip_region.y=0;
            clip_region.w=e->render_size.x;
            clip_region.h=e->render_size.y;

            r.w=size_width;
            r.h=size_height;   
            r.x=e->render_position.x;
            r.y=e->render_position.y+size_height*i;
            
            draw_texture(tbr->w, tbr->font_textures[i], &r, e->render_angle, NULL, NULL, tbr->do_clip ? &clip_region : 0);
        }
    }
}
text_block_renderer *ctor_text_block_renderer(window *w, ttf_font *font, bool do_clip, u32 *line_numbers, char *alignment)
{
    text_block_renderer *tbr=(text_block_renderer*)mem_alloc(sizeof(text_block_renderer));
    tbr->r.render=text_block_renderer_render;
    tbr->line_numbers=line_numbers;
    tbr->do_clip=do_clip;
    tbr->f=font;
    tbr->w=w;
    tbr->font_textures=NULL;
    tbr->line_number_textures=NULL;
    tbr->text=NULL;
    tbr->lines=0;
    tbr->alignment=alignment;
    return tbr;
}
void text_block_renderer_set_text(text_block_renderer *t, char **text, u32 lines, color text_color, u32 *line_to_rerender)
{
    u32 i;

    /*if the array changes length we rerender (recreate) everything, otherwise its on you to tell when to recreate everything*/
    /*@perf instead of rerendering everything if lines != t->lines just rerender latest one? and count on them to rerender everything if they inserted or something?*/
    if(!line_to_rerender || lines != t->lines)
    {
	/*
	make sure array is initialized and array elements are zero initialized
	*/
        if(!t->font_textures)
        {
            t->font_textures=(texture**)mem_alloc(lines * sizeof(texture*));
            zero_out(t->font_textures,lines * sizeof(texture*));
            if(*t->line_numbers)
            {
                t->line_number_textures=(texture**)mem_alloc(lines * sizeof(texture*));
	        zero_out(t->line_number_textures,lines * sizeof(texture*));
            }
        }
        else
        {
            for(i=0; i<t->lines; i++)
            {
                dtor_texture(t->font_textures[i]);
            }
            mem_free(t->font_textures);

            t->font_textures=(texture**)mem_alloc(lines * sizeof(texture*));
            zero_out(t->font_textures,lines * sizeof(texture*));

            if(*t->line_numbers)
            {
                if(t->line_number_textures)
                {
                    for(i=0; i<t->lines; i++)
                    {
                        dtor_texture(t->line_number_textures[i]);
                    }
                    mem_free(t->line_number_textures);   
                }
                t->line_number_textures=(texture**)mem_alloc(lines * sizeof(texture*));
                zero_out(t->line_number_textures,lines * sizeof(texture*));
            }
        }

        for(i=0; i<lines; i++)
        {
            if(t->font_textures[i])
            {
                dtor_texture(t->font_textures[i]);
            }

            /*@bug
            I had to add this check for empty string otherwise it was failing to create texture,
            could be what is causing cursor to be moving to beginning?
            */
            if(text[i][0]==0){
                t->font_textures[i]=NULL;
            }
            else{
                t->font_textures[i]=ctor_texture_font(t->w,t->f,text[i],text_color);
            }
        
            if(*t->line_numbers)
            {
                if(t->line_number_textures[i])
                {
                    dtor_texture(t->line_number_textures[i]);
                }
                char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
                itoa(i+1,buffer,10);
                t->line_number_textures[i]=ctor_texture_font(t->w,t->f,buffer,text_color);
                texture_set_alpha(t->line_number_textures[i],100);
            }
        }
    }
    else
    {
        if(t->font_textures[*line_to_rerender])dtor_texture(t->font_textures[*line_to_rerender]);
        if(text[*line_to_rerender][0]){
            t->font_textures[*line_to_rerender]=ctor_texture_font(t->w,t->f,text[*line_to_rerender],text_color);
        }

        if(*t->line_numbers)
        {
            dtor_texture(t->line_number_textures[*line_to_rerender]);
            char buffer[MAXIMUM_LINE_NUMBER_LENGTH];
            itoa((*line_to_rerender)+1,buffer,10);
            t->line_number_textures[*line_to_rerender]=ctor_texture_font(t->w,t->f,buffer,text_color);
            texture_set_alpha(t->line_number_textures[*line_to_rerender],100);
        }
    }

    t->text=text;
    t->lines=lines;
}
/*doesnt mem_free the font, you have to manage that separately (keep a reference)*/
void dtor_text_block_renderer(text_block_renderer *t)
{
    u32 i;
    
    if(t->font_textures)
    {
        for(i=0; i<t->lines; i+=1)
        {
            if(t->font_textures[i])dtor_texture(t->font_textures[i]);
        }
        
        mem_free(t->font_textures);
    }
    if(t->line_number_textures)
    {
        for(i=0; i<t->lines; i+=1)
        {
            if(t->line_number_textures[i])dtor_texture(t->line_number_textures[i]);
        }
        mem_free(t->line_number_textures);
    }
    mem_free(t);
}

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

typedef struct image_renderer
{
    renderer r;
    texture *image_texture;
    window *w;
    rect *clip_region;
} image_renderer;
void image_renderer_render(entity *e, renderer *i_r)
{
    image_renderer *i=(image_renderer*)i_r;
    rect r;
    r.x=e->render_position.x;
    r.y=e->render_position.y;
    r.w=e->render_size.x;
    r.h=e->render_size.y;

    texture_set_alpha(i->image_texture,e->render_alpha*255);
    draw_texture(i->w, i->image_texture, &r, e->render_angle, NULL, NULL, i->clip_region);
}
image_renderer *ctor_image_renderer(window *w,texture *t, rect *clip_region)
{
    image_renderer *img=(image_renderer*)mem_alloc(sizeof(image_renderer));
    img->w=w;
    img->image_texture=t;
    img->r.render=image_renderer_render;
    img->clip_region=clip_region;
    return img;
}
void image_renderer_set_texture(image_renderer *img, texture *t)
{
    img->image_texture=t;
}
void dtor_image_renderer(image_renderer *img)
{
    dtor_texture(img->image_texture);
    mem_free(img);
}