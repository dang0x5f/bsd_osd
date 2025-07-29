#ifndef OSD_OUTMIXER_H
#define OSD_OUTMIXER_H

#include <sys/sysctl.h>
#include <mixer.h>

#define OSD_BTTN_IMPLEMENTATION
#include "osd_button.h"

int osd_outmixer(void);         // Driver function
void *create_mixerlist(void);   // Retrieve list of mixer devices

typedef osd_button Button;

typedef struct node_t{
    Window win_id;
    int mixer_id;
    Button *btn;
    struct node_t *next;
    struct node_t *prev;
} Button_node;

typedef struct {
    Button_node *first;
    Button_node *end;
    int default_mixer;
    Button_node *current_mixer;
    size_t length;
} Button_List;

typedef struct  {
    Display *display;
    int screen_num;
    Colormap colormap;
    int depth;
    Visual *visual;
    XSetWindowAttributes attributes;
    int valuemask;
    int width;
    int height;
} WinResources;

typedef struct {
    Window win_id;
    Button_List list;
} UnitData;

#endif

#ifdef OSD_OUTMIXER_IMPLEMENTATION

#define XPOS 0
#define YPOS 0
#define BLACK "#000000"
#define RED40 "#CE2525"
#define INDICATOR_COLOR RED40

WinResources *init_resources(void);    // Setup window essentials
Button_List create_buttonlist(WinResources*,Window,XContext*,int,int,int,XftFont*);   
void get_mixer_info(size_t*,size_t*);
int get_defaultunit(void);     
Window get_defaultunit_window(Button_List*);
void set_defaultunit2(int,Button_List*);
void init_selected_mixer(WinResources*,Button_List*);
XftGlyphFontSpec *init_default_indicator(WinResources*,XftFont*,uint32_t);
void indicator_setup(WinResources*,XftFont*);
bool process_keypress(WinResources*,Button_List*,KeySym);
void draw_buttons(WinResources*,Button_List*,Button_node*);

/* TODO: struct */
/* include size for draw call */ 
XftColor indic_color;
XftGlyphFontSpec *indicator;
const uint32_t border_pixel = 2;
const uint32_t vert_between_pad = 3;
const char *font_name = "Deja Vu Sans Mono:pixelsize=16";

int osd_outmixer(void)
{
    XEvent ev;
    Button_List button_list;
    bool running=true;
    size_t nmixers, max_name_len;
    int32_t x_pos = 0; 
    int32_t y_pos = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t width_pad = 0;
    uint32_t button_width = 0;
    uint32_t button_height = 0;

    XContext context = XUniqueContext();
    WinResources *R  = init_resources();
    XftFont *font    = font_setup(R->display,R->screen_num,font_name);
    Window root      = DefaultRootWindow(R->display);

    indicator_setup(R,font);
    get_mixer_info(&nmixers,&max_name_len);

    width_pad = font->max_advance_width;
    button_width  = (font->max_advance_width*max_name_len)+(width_pad*2);
    button_height = (font->ascent+font->descent);
    width  = (button_width)+(border_pixel*2)+(vert_between_pad*2);
    height = ((button_height+(border_pixel*2))*(nmixers))+(vert_between_pad*(nmixers+1));
    y_pos  = DisplayHeight(R->display,R->screen_num)-(height+(border_pixel*2));
    
    Window window = XCreateWindow(R->display,root,x_pos,y_pos,width,height,
                                  border_pixel,R->depth,CopyFromParent,
                                  R->visual,R->valuemask,&R->attributes);

    button_list = create_buttonlist(R,window,&context,button_width,
                                    button_height,nmixers,font);

    init_selected_mixer(R,&button_list);

    XMapWindow(R->display,window);
    XSync(R->display,false);

    XGrabKeyboard(R->display,window,true,GrabModeAsync,GrabModeAsync,CurrentTime);

    while(running){
        osd_button *btn = NULL;
        XNextEvent(R->display,&ev);
        XFindContext(ev.xany.display,ev.xany.window,context,(XPointer*)&btn);
        switch(ev.type){
            case ConfigureNotify:
                if(btn) config_button(btn,&ev);
                break;
            case Expose:
                if(btn){
                    expose_button(btn,&ev);
                    if(btn->win == get_defaultunit_window(&button_list)){
                        XftDrawGlyphFontSpec(btn->draw,&indic_color,indicator,1);
                    }
                } 
                break;
            case KeyPress:
                KeySym keysym = XLookupKeysym(&ev.xkey,0);
                running = process_keypress(R,&button_list,keysym);
                break;
        }
    }

    XUngrabKeyboard(R->display,CurrentTime);

    return(EXIT_SUCCESS);
}


WinResources *init_resources(void)
{
    WinResources *res = malloc(sizeof(WinResources));

    res->display    = XOpenDisplay(NULL);
    res->screen_num = DefaultScreen(res->display);
    res->colormap   = DefaultColormap(res->display,res->screen_num);
    res->depth      = DefaultDepth(res->display,res->screen_num);
    res->visual     = DefaultVisual(res->display,res->screen_num);

    res->attributes.override_redirect = true;
    res->attributes.background_pixel  = 0x555555;
    res->attributes.border_pixel      = 0xfffdd0;
    res->attributes.event_mask = ExposureMask
                               | KeyPressMask
                               | VisibilityChangeMask
                               | SubstructureNotifyMask;
    res->valuemask = CWOverrideRedirect
                   | CWBackPixel
                   | CWEventMask
                   | CWBorderPixel;

    return(res);
}

Button_List create_buttonlist(WinResources *R, Window parent, XContext *context, 
                              int width, int height, int nmixers, XftFont *font)
{
    Button_List list = { .first = NULL, .length = 0 };
    list.default_mixer = get_defaultunit();
    Button_node *node;

    Button *btn;
    Window subwin;
    struct mixer *m;
    char buffer[NAME_MAX];
    char* fg_color = BLACK;

    for(int i=0; i<nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);
        if((m=mixer_open(buffer))==NULL) continue;

        size_t name_len = strlen(m->ci.longname);
        char *name = malloc(sizeof(char)*name_len);
        strncpy(name,m->ci.longname,name_len);
        btn = create_button(R->display, &parent, &subwin, R->depth, R->visual, 
                            *context, vert_between_pad, 
                            ((height+(border_pixel*2))*i)+(vert_between_pad*i)+vert_between_pad, 
                            width, height, &R->colormap, border_pixel,0x333333, 0xbbbbbb, 
                            fg_color, name, name_len, NULL, font);

        /* TODO: free */
        node = malloc(sizeof(Button_node));
        node->win_id = subwin;
        node->mixer_id = i;
        node->btn = btn;
        node->next = NULL;
        node->prev = NULL;

        if(list.first == NULL){
            list.first = node;
        }else{
            Button_node *iter = list.first;
            while(iter->next)
                iter = iter->next;
            iter->next = node;
            node->prev = iter;
        }

        if(list.default_mixer==i)
            list.current_mixer = node;

        list.length += 1;

        (void)mixer_close(m);
    }    

    // wrap next of end to first of list
    node->next = list.first;
    // wrap prev of first to end of list
    list.first->prev = node;
    list.end = node;

    return(list);
}

void init_selected_mixer(WinResources *R, Button_List *list)
{
    Button_node *iter = list->first;
    while(1){
        if(iter->mixer_id == list->current_mixer->mixer_id){
            select_button(iter->btn,R->display,iter->win_id);
            break;
        } 
        iter = iter->next;
    }
}

void get_mixer_info(size_t *nmixers, size_t *max_name_len)
{
    *nmixers = 0;
    *max_name_len = 0;

    if((*nmixers=mixer_get_nmixers())<0)
        errx(1,"No mixers present in system");
    
    struct mixer *m;
    char buffer[NAME_MAX];

    for(size_t i=0; i<*nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        if(strlen(m->ci.longname)>*max_name_len){
            *max_name_len = MAX(*max_name_len,strlen(m->ci.longname));
        }
         
        (void)mixer_close(m);
    }    
    --(*nmixers);
}

int get_defaultunit(void)
{
    int input = 0;
    size_t ilen = sizeof(input);
    if(sysctlbyname("hw.snd.default_unit",&input,&ilen,NULL,0)==-1){
        perror("sysctlbyname() error\n");
        exit(EXIT_FAILURE);
    }
    return(input);
}

Window get_defaultunit_window(Button_List *list)
{
    Button_node *iter = list->first;
    for(size_t i=0;i<list->length;++i){
        if(iter->mixer_id == list->default_mixer)
            return(iter->win_id);
        iter = iter->next;
    }
    return(list->first->win_id);
}

// TODO: rename appropriately
void set_defaultunit2(int mixer_id,Button_List *list)
{
    /* TODO: alter for virtual_oss , call exec() */

    int input,output=mixer_id;
    size_t input_len=sizeof(input),output_len=sizeof(output);
    sysctlbyname("hw.snd.default_unit",&input,&input_len,&output,output_len);

    list->default_mixer = mixer_id;

    printf("%d\n", mixer_id);
}

XftGlyphFontSpec *
init_default_indicator(WinResources *R, XftFont *font, uint32_t glyph)
{
    XftGlyphFontSpec *indicator_spec = malloc(sizeof(XftGlyphFontSpec));
    if(indicator_spec==NULL){
        printf("init_default_indicator() error : malloc\n");
        exit(EXIT_FAILURE);
    }
    
    uint32_t index = XftCharIndex(R->display,font,glyph);
    if(index){
        indicator_spec->font = font;
        indicator_spec->glyph = index;
        indicator_spec->x = 0;
        indicator_spec->y = font->ascent;
    }else{
        printf("init_default_indicator() error : no index\n");
        exit(EXIT_FAILURE);
    }

    return(indicator_spec);
}

void draw_buttons(WinResources *R, Button_List *list, Button_node *node)
{
    Button_node *def_mixer = list->first;

    for(size_t i=0; i<list->length; ++i){
        if(node->mixer_id == list->current_mixer->mixer_id)
            select_button(node->btn,R->display,node->win_id);
        else
            unselect_button(node->btn,R->display,node->win_id);
        
        if(node->mixer_id == list->default_mixer) def_mixer = node;

        node = node->next;
    }
    XftDrawGlyphFontSpec(def_mixer->btn->draw,&indic_color,indicator,1);
}

bool process_keypress(WinResources *R, Button_List *list, KeySym keysym)
{
    bool running = true;
    Button_node *node;
    node = list->first;

    switch(keysym){
        case XK_j:
            list->current_mixer = list->current_mixer->next;
            break;
        case XK_k:
            list->current_mixer = list->current_mixer->prev;
            break;
        case XK_Return:
            /* XClearWindow(R->display,list->current_mixer->win_id); */
            set_defaultunit2(list->current_mixer->mixer_id,list);
            break;
        case XK_q:
        case XK_Escape:
            running=false;
            break;
    }
    draw_buttons(R,list,node);

    return(running);
}

void indicator_setup(WinResources *R,XftFont *font)
{
    uint32_t glyph = 0x2023;
    indicator = init_default_indicator(R, font, glyph);
    XftColorAllocName(R->display,R->visual,R->colormap,INDICATOR_COLOR,&indic_color);
}

#endif
