#include "../h/syntax_highlight.h"

#include "opl/src/h/system.h"

color get_field(char *field, ovp *config, color defaultc){
    color retval;
    /*printf("getting color for:%s\n",field);*/
    
    for(int i=0; i<config->identifiers_size; i++){   
        if(str_eq(field,config->identifiers[i])){
            if(config->identifiers_data[i].strs_size<3){
                continue;
            }
            retval.r=atoi(config->identifiers_data[i].strs[0]);
            retval.g=atoi(config->identifiers_data[i].strs[1]);
            retval.b=atoi(config->identifiers_data[i].strs[2]);
            retval.a=255;
            /*printf("found for:%s\n",field);*/
            return retval;
        }
    }
    return defaultc;
}
color get_custom_color(char *ext, char *token, ovp *config, color defaultc){
    
    color retval;
    if(!ext){
        return defaultc;
    }
    char *a=str_cat(ext,"_custom_");
    char *b=str_cat(a,token);
    free(a);
    retval=get_field(b,config,defaultc);
    free(b);
    return retval;
}

void init_line_textures(line_textures *lt, window *w, char *line, ovp *config, ttf_font *f, char *ext){
    
    color hard_default;
    hard_default.r=255;
    hard_default.g=255;
    hard_default.b=255;
    hard_default.a=255;

    lt->textures_sz=0;
    lt->textures=NULL;
    lt->textures_rects=NULL;

    if(!ext || !ext[0]){
        /*no extension, no highlight*/
        lt->textures_sz=1;
        lt->textures=mem_alloc(sizeof(texture*)*lt->textures_sz);
        lt->textures_rects=mem_alloc(sizeof(rect)*lt->textures_sz);

        lt->textures[0]=ctor_texture_font(w,f,line,hard_default);
        int size_x,size_y;
        size_ttf_font(f,line,&size_x,&size_y);

        lt->textures_rects[0].x=0;
        lt->textures_rects[0].y=0;
        lt->textures_rects[0].w=size_x;
        lt->textures_rects[0].h=size_y;

        return;
    }

    char *defstr=str_cat(ext,"_default");
    char *operatorsstr=str_cat(ext,"_operators");
    char *numbersstr=str_cat(ext,"_numbers");
    char *stringsstr=str_cat(ext,"_strings");
    
    color defc;
    color operatorsc;
    color numbersc;
    color stringsc;

    defc=get_field(defstr,config,hard_default);
    
    operatorsc=get_field(operatorsstr,config,defc);
    numbersc=get_field(numbersstr,config,defc);
    stringsc=get_field(stringsstr,config,defc);

    /*format
    <ext>_default : r g b
    <ext>_operators : r g b
    <ext>_numbers : r g b
    <ext>_strings : r g b
    <ext>_custom_<str> : r g b
    */

    /*
    separators are operators, spaces, newlines (eos)
        operators themselves have their own color
    then can find if its a string or number
    otherwise finally check for custom
    */
    char **tokens=NULL;
    int tokens_sz=0;
    char *operators=" [](){}.,></?|;:!@#$%^&*-=+";/*\ (escape) not included nor are ' " strings handled after*/

    int n;
    int line_len=strlen(line);
    
    for(n=0;n<line_len;n++){
        
        /*
        @todo comments (handling multiline comments wont be possible w/ line by line)
            multiline strings, or preprocessor, or anything else, same issue
            would have to handle outside, best way would just be to check every time someone types / or * or / and then do a large rerender from outside
                if /* recreate textures until hit closer, have to check for delete and search backwards for closer or opener
                    potentially very slow (testing in vscode there is some wonkiness where it rerenders asynchronously in large blocks)
        start by turning line into tokens (dont worry about config at all here)
        */

        /**/
        int begin=n;
        if(line[n]=='"' || line[n]=='\''){
            /*looking for strings gets first priority*/
            n++;
            while(n<line_len){
                if(line[n]==line[begin] && line[n-1]!='\\'){
                    n++;
                    break;
                }
                n++;
            }
        }
        else if(str_contains(operators,line[n])){
            /*then separate things by operators (incl space)*/
            while(str_contains(operators,line[n]) && line[n]){
                n++;
            }
        }
        else{
            /*otherwise its something else*/
            while(!str_contains(operators,line[n]) && line[n]){
                n++;
            }
        }
        n--;/*stupid inclusive string function why did i do this*/
        char *newtoken=alloc_str_slice(line,begin,n);
       
        if(newtoken && strlen(newtoken)){
            tokens_sz++;
            /*add token*/
            tokens=realloc(tokens,tokens_sz*sizeof(char*));
            tokens[tokens_sz-1]=newtoken;
        }
    }
    
    int j;
    for(j=0;j<tokens_sz;j++){
        /*operatorsc, numbersc, stringsc, otherwise lookup*/
        bool is_operator=false;
        bool is_number=true;
        bool is_string=false;

        int token_len=strlen(tokens[j]);
        
        int k=0;
        /*is operator*/
        while(tokens[j][k]){
            if(str_contains(operators,tokens[j][k])){
                is_operator=true;
                break;
            }
            k++;
        }

        k=0;
        /*is number*/
        while(tokens[j][k]){
            if(!(tokens[j][k] > 47 && tokens[j][k] < 58)){
                is_number=false;
            }
            k++;
        }
        
        /*is str*/
        if(tokens[j][0] == '\"' || tokens[j][0] == '\'')
        {
            if(tokens[j][token_len-1]==tokens[j][0]){
                is_string=true;
            }
        }
        
        lt->textures_sz++;
        lt->textures=realloc(lt->textures,sizeof(texture*)*lt->textures_sz);
        lt->textures_rects=realloc(lt->textures_rects,sizeof(rect)*lt->textures_sz);

        if(is_string){
            lt->textures[lt->textures_sz-1]=ctor_texture_font(w,f,tokens[j],stringsc);
        }
        else if(is_operator){
            lt->textures[lt->textures_sz-1]=ctor_texture_font(w,f,tokens[j],operatorsc);
        }
        else if(is_number){
            lt->textures[lt->textures_sz-1]=ctor_texture_font(w,f,tokens[j],numbersc);
        }
        else{/*custom*/
            lt->textures[lt->textures_sz-1]=ctor_texture_font(w,f,tokens[j],get_custom_color(ext,tokens[j],config,defc));    
        }

        int size_x,size_y;
        size_ttf_font(f,tokens[j],&size_x,&size_y);
        lt->textures_rects[lt->textures_sz-1].x=0;
        lt->textures_rects[lt->textures_sz-1].y=0;
        lt->textures_rects[lt->textures_sz-1].w=size_x;
        lt->textures_rects[lt->textures_sz-1].h=size_y;
    }

    for(j=0; j<tokens_sz; j++){
        free(tokens[j]);
    }
    free(tokens);

    free(defstr);
    free(operatorsstr);
    free(numbersstr);
    free(stringsstr);
}

void release_line_textures(line_textures *lt){
    if(lt->textures_sz){/*has been initialized*/
        for(int i=0; i<lt->textures_sz; i++){
            dtor_texture(lt->textures[i]);
        }
        free(lt->textures);
        free(lt->textures_rects);
    }
}

/*  
    printf("got config: \n");
    for(int i=0; i<config->identifiers_size; i++){
        printf("line: %s\n",config->identifiers[i]);
        for(int j=0; j<config->identifiers_data[i].strs_size; j++){
            printf("param %d: %s\n",j,config->identifiers_data[i].strs[j]);
        }
    }
    */
    
    /*@temp just construct a single white line for now
    lt->textures_sz=1;
    lt->textures=mem_alloc(sizeof(texture*)*lt->textures_sz);
    lt->textures_rects=mem_alloc(sizeof(rect)*lt->textures_sz);

    lt->textures[0]=ctor_texture_font(w,f,line,value_color(255,0,255,255));
    int size_x,size_y;
    size_ttf_font(f,line,&size_x,&size_y);
    lt->textures_rects[0].x=0;
    lt->textures_rects[0].y=0;
    lt->textures_rects[0].w=size_x;
    lt->textures_rects[0].h=size_y;
    */

    /*simpler test for changing color and splitting into multiple
    lt->textures_sz=0;
    lt->textures=NULL;
    lt->textures_rects=NULL;
    bool found=false;
    int strlenline=strlen(line);
    for(int i=0; line[i]; i++){
        if(strlenline-(i+2) > 0 && line[i]=='i'&&line[i+1]=='n'&&line[i+2]=='t'){
            lt->textures_sz=3;
            
            lt->textures=mem_alloc(sizeof(texture*)*lt->textures_sz);
            lt->textures_rects=mem_alloc(sizeof(rect)*lt->textures_sz);

            char *s0,*s1,*s2;
            s0=alloc_str_slice(line,0,i-1);
            s1=alloc_str_slice(line,i,i+2);
            s2=alloc_str_slice(line,i+3,strlenline);
            int size_x,size_y;
            
            size_ttf_font(f,s0,&size_x,&size_y);
            lt->textures[0]=ctor_texture_font(w,f,s0,value_color(255,0,255,255));
            lt->textures_rects[0].x=0;
            lt->textures_rects[0].y=0;
            lt->textures_rects[0].w=size_x;
            lt->textures_rects[0].h=size_y;

            size_ttf_font(f,s1,&size_x,&size_y);
            lt->textures[1]=ctor_texture_font(w,f,s1,value_color(0,0,255,255));
            lt->textures_rects[1].x=0;
            lt->textures_rects[1].y=0;
            lt->textures_rects[1].w=size_x;
            lt->textures_rects[1].h=size_y;

            size_ttf_font(f,s2,&size_x,&size_y);
            lt->textures[2]=ctor_texture_font(w,f,s2,value_color(255,0,255,255));
            lt->textures_rects[2].x=0;
            lt->textures_rects[2].y=0;
            lt->textures_rects[2].w=size_x;
            lt->textures_rects[2].h=size_y;
            
            free(s0);
            free(s1);
            free(s2);
            found=true;
            break;
        }
    }
    if(!found){
        lt->textures_sz=1;
    lt->textures=mem_alloc(sizeof(texture*)*lt->textures_sz);
    lt->textures_rects=mem_alloc(sizeof(rect)*lt->textures_sz);

    lt->textures[0]=ctor_texture_font(w,f,line,value_color(255,0,255,255));
    int size_x,size_y;
    size_ttf_font(f,line,&size_x,&size_y);
    lt->textures_rects[0].x=0;
    lt->textures_rects[0].y=0;
    lt->textures_rects[0].w=size_x;
    lt->textures_rects[0].h=size_y;
    }
    */