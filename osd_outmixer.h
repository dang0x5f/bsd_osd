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
} Button_node;

typedef struct {
    Button_node *first;
    int default_mixer;
    int current_mixer;
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
#define WIDTH 300
#define HEIGHT 400
#define BORDER_PIXEL 2
#define BLACK "#000000"
#define GREEN "#00a500"
#define LIGHT_GREEN "#90ee90"
#define DARK_GREEN "#026440"

WinResources *init_resources(void);    // Setup window essentials
Button_List create_buttonlist(WinResources*,Window,XContext*,int,int,int,XftFont*);   
void get_mixer_info(size_t*,size_t*);
int get_defaultunit(void);     
void set_defaultunit(void*);
void set_defaultunit2(int,Button_List*);
void reassign_foreground(WinResources*,int, Button_List*);
void init_selected_mixer(WinResources*,Button_List*);

char *font_name = "Deja Vu Sans Mono:pixelsize=16";


int osd_outmixer(void)
{
    XContext context = XUniqueContext();

    WinResources *R = init_resources();

    XftFont *font = font_setup(R->display,R->screen_num,font_name);

    size_t nmixers, max_name_len;
    get_mixer_info(&nmixers,&max_name_len);
    printf("%zu\n",nmixers);

    int ypadding = 3;
    int padding = 5;

    int width = font->max_advance_width * max_name_len+(BORDER_PIXEL*2) + (padding*2);
    int height = ((font->ascent+font->descent+(BORDER_PIXEL*2))*(nmixers-1));

    height += (ypadding*(nmixers-2));

    int xpos = DisplayWidth(R->display,R->screen_num);
    int ypos = DisplayHeight(R->display,R->screen_num);
    ypos = (ypos/2)-(height/2);
    
    Window root = DefaultRootWindow(R->display);
    Window window = XCreateWindow(R->display,root,XPOS,ypos,width,height,
                                  BORDER_PIXEL,R->depth,CopyFromParent,
                                  R->visual,R->valuemask,&R->attributes);

    int width1 =  font->max_advance_width * max_name_len;
    /* TODO: maybe add vertical padding */
    int height1 = font->ascent+font->descent;

    Button_List button_list;
    button_list = create_buttonlist(R,window,&context,width1,height1,nmixers,font);
    init_selected_mixer(R,&button_list);

    XMapWindow(R->display,window);
    XSync(R->display,false);
    XGrabKeyboard(R->display,window,true,GrabModeAsync,GrabModeAsync,CurrentTime);

    bool running=true;
    XEvent ev;
    while(running){
        Button_node *node = NULL;
        osd_button *btn = NULL;
        XNextEvent(R->display,&ev);
        XFindContext(ev.xany.display,ev.xany.window,context,(XPointer*)&btn);
        switch(ev.type){
            case ConfigureNotify:
                if(btn) config_button(btn,&ev);
                break;
            case Expose:
                if(btn) expose_button(btn,&ev);
                break;
            /* case EnterNotify: */
            /*     if(btn) enter_button(btn,&ev); */
            /*     break; */
            /* case LeaveNotify: */
            /*     if(btn) leave_button(btn,&ev); */
            /*     break; */
            case ButtonRelease:
                if(btn){
                    UnitData cbdata = { 
                        .win_id = ev.xany.window, 
                        .list = button_list 
                    };
                    btn->buttonRelease(&cbdata);
                }
                break;
            case KeyPress:
                KeySym keysym = XLookupKeysym(&ev.xkey,0);
                /* if(keysym==XK_Escape || keysym==XK_q) running=false; */
                switch(keysym){
                    case XK_j:
                        button_list.current_mixer += 1;
                        if(button_list.current_mixer==button_list.length) 
                            button_list.current_mixer = 0;
                        node = button_list.first;
                        for(int i=0; i<button_list.length; ++i){
                            bool invert = true;
                            if(node->mixer_id == button_list.current_mixer){
                                if(node->mixer_id == button_list.default_mixer)
                                    invert=false;
                                select_button(node->btn,R->display,node->win_id,invert);
                            } else{
                                unselect_button(node->btn,R->display,node->win_id);
                            }

                            node = node->next;
                        }
                        break;
                    case XK_k:
                        button_list.current_mixer -= 1;
                        if(button_list.current_mixer<0) 
                            button_list.current_mixer = button_list.length-1;
                        node = button_list.first;
                        for(int i=0; i<button_list.length; ++i){
                            bool invert = true;
                            if(node->mixer_id == button_list.current_mixer){
                                if(node->mixer_id == button_list.default_mixer)
                                    invert=false;
                                select_button(node->btn,R->display,node->win_id,invert);
                            }
                            else{
                                unselect_button(node->btn,R->display,node->win_id);
                            }

                            node = node->next;
                        }
                        break;
                    case XK_Return:
                        set_defaultunit2(button_list.current_mixer,&button_list);
                        reassign_foreground(R,button_list.current_mixer,&button_list);
                        node = button_list.first;
                        for(int i=0; i<button_list.length; ++i){
                            bool invert = true;
                            if(node->mixer_id == button_list.current_mixer){
                                if(node->mixer_id == button_list.default_mixer)
                                    invert=true;
                                select_button(node->btn,R->display,node->win_id,invert);
                            } else{
                                unselect_button(node->btn,R->display,node->win_id);
                            }

                            node = node->next;
                        }
                        break;
                    case XK_q:
                    case XK_Escape:
                        running=false;
                        break;
                }
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
    res->attributes.background_pixel  = 0xffffff;
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
    list.current_mixer = list.default_mixer;

    Button *btn;
    Window subwin;
    int ypad = 3;
    struct mixer *m;
    char buffer[NAME_MAX];

    char* fg_color;

    for(int i=0; i<nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        fg_color = (list.default_mixer==i?GREEN:BLACK);
        size_t name_len = strlen(m->ci.longname);
        char *name = malloc(sizeof(char)*name_len);
        strncpy(name,m->ci.longname,name_len);
        btn = create_button(R->display, &parent, &subwin, R->depth, R->visual, 
                            *context, 0, ((height+(BORDER_PIXEL*2))*i)+(ypad*i), width, &R->colormap, 
                            BORDER_PIXEL,0x333333, 0xbbbbbb, fg_color, name, 
                            name_len, set_defaultunit, font);

        /* TODO: free */
        Button_node *node = malloc(sizeof(Button_node));
        node->win_id = subwin;
        node->mixer_id = i;
        node->btn = btn;
        node->next = NULL;

        if(list.first == NULL){
            list.first = node;
        }else{
            Button_node *iter = list.first;
            while(iter->next)
                iter = iter->next;
            iter->next = node;
        }

        list.length += 1;

        (void)mixer_close(m);
    }    

    return(list);
}

/* TODO: FREE COLORS, create color system */
/* void */
/* XftColorFree (Display	*dpy, */
/* 	      Visual	*visual, */
/* 	      Colormap	cmap, */
/* 	      XftColor	*color); */

void reassign_foreground(WinResources *R, int new_defaultunit, Button_List *list)
{
    Button_node *node = list->first;


    XftColor def_color;
    XftColorAllocName(R->display,R->visual,R->colormap,GREEN,&def_color);

    XftColor norm_color;
    XftColorAllocName(R->display,R->visual,R->colormap,BLACK,&norm_color);
    XftColor inverted_color;
    char inverted[8]={'\0'};
    invert_color(BLACK,inverted);
    XftColorAllocName(R->display,R->visual,R->colormap,inverted,&inverted_color);

    while(node){
        if(node->mixer_id == new_defaultunit){
            node->btn->fg = def_color;
            node->btn->inverted_fg = def_color;
        }
        else{
            node->btn->fg = norm_color;
            node->btn->inverted_fg = inverted_color;
        }
        
        node = node->next;
    }
}

void init_selected_mixer(WinResources *R, Button_List *list)
{
    Button_node *iter = list->first;
    while(1){
        if(iter->mixer_id == list->current_mixer){
            XSetWindowAttributes attributes;
            attributes.background_pixel = iter->btn->border;
            attributes.border_pixel = iter->btn->background;
            (iter->btn->foreground) = (iter->btn->inverted_fg);
            XChangeWindowAttributes(R->display, iter->win_id,
                                    CWBackPixel|CWBorderPixel, &attributes);
            XClearArea(R->display, iter->win_id, 0,0, iter->btn->width,
                       iter->btn->height, True);
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
}

int get_defaultunit(void)
{
    int input;
    size_t ilen = sizeof(input);
    // TODO: errno str
    if(sysctlbyname("hw.snd.default_unit",&input,&ilen,NULL,0)==-1){
        perror("sysctlbyname() error\n");
        exit(EXIT_FAILURE);
    }
    return(input);
}

void set_defaultunit2(int mixer_id,Button_List *list)
{
    /* TODO: alter for virtual_oss , call exec() */

    int input,output=mixer_id;
    size_t input_len=sizeof(input),output_len=sizeof(output);
    sysctlbyname("hw.snd.default_unit",&input,&input_len,&output,output_len);

    list->default_mixer = mixer_id;

    printf("%d\n", mixer_id);
}

void set_defaultunit(void *unitdata)
{
    UnitData *ud = (UnitData*)unitdata;
    Window win_id = ud->win_id;
    Button_List list = ud->list;

    Button_node *node = list.first;
    while(node->next){
        if(node->win_id == win_id) break;
        
        node = node->next;
    }

    int input,output=node->mixer_id;
    size_t input_len=sizeof(input),output_len=sizeof(output);
    sysctlbyname("hw.snd.default_unit",&input,&input_len,&output,output_len);

    printf("%d\n", node->mixer_id);
}

#endif
