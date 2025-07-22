#ifndef OSD_OUTMIXER_H
#define OSD_OUTMIXER_H

#include <sys/sysctl.h>
#include <mixer.h>

#define OSD_BTTN_IMPLEMENTATION
#include "osd_button.h"

int osd_outmixer(void);         // Driver function
void *create_mixerlist(void);   // Retrieve list of mixer devices
void get_defaultunit(void);     
void set_defaultunit(void);

typedef osd_button Button;

typedef struct node_t{
    Window win_id;
    int mixer_id;
    struct node_t *next;
} Button_node;

typedef struct {
    Button_node *first;
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

#endif

#ifdef OSD_OUTMIXER_IMPLEMENTATION

#define XPOS 0
#define YPOS 0
#define WIDTH 300
#define HEIGHT 400
#define BORDER_PIXEL 2

WinResources *init_resources(void);    // Setup window essentials
Button_List create_buttonlist(WinResources*,Window,XContext*,int,int,int,XftFont*);   
char* get_mixer_info(size_t*,size_t*);

char *font_name = "Deja Vu Sans Mono:pixelsize=20";


int osd_outmixer(void)
{
    XContext context = XUniqueContext();

    WinResources *R = init_resources();

    XftFont *font = font_setup(R->display,R->screen_num,font_name);

    size_t nmixers, max_name_len;
    char *name = get_mixer_info(&nmixers,&max_name_len);
    printf("%zu\n",nmixers);
    /* printf("%s\n",name); */
    /* if((nmixers=mixer_get_nmixers())<0) */
    /*     errx(1,"No mixers present in system"); */
    /* printf("%d\n",max_name_len); */



    /* int height = (nmixers*2)*font->height; */


    int width = font->max_advance_width * max_name_len+(BORDER_PIXEL*2);
    int height = (font->ascent+font->descent+(BORDER_PIXEL*2)+(BORDER_PIXEL*2))*(nmixers-1);

    
    Window root = DefaultRootWindow(R->display);
    /* TODO: make less messy, and add remarks why */
    // width  + (BORDER*2)                  -  account for button border
    // height + (BORDER*2) + (BORDER*2)     -  account for button border .. 2 times works?
    Window window = XCreateWindow(R->display,root,XPOS,YPOS,width,height,
                                  BORDER_PIXEL,R->depth,CopyFromParent,
                                  R->visual,R->valuemask,&R->attributes);

    int width1 =  font->max_advance_width * max_name_len;
    int height1 = font->ascent+font->descent+(BORDER_PIXEL*2)+(BORDER_PIXEL*2);
        /* Window subwin = create_button(R->display, &window, R->depth, R->visual, */ 
        /*                               context, 0,0, width1, &R->colormap, */ 
        /*                               0xffaa5f, 0x000000, "#00FF00", name, */ 
        /*                               max_name_len,  NULL, font); */
    Button_List button_list;
    button_list = create_buttonlist(R,window,&context,width1,height1,nmixers,font);

    XMapWindow(R->display,window);
    XSync(R->display,false);
    
    XEvent ev;
    while(1){
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
            case EnterNotify:
                if(btn) enter_button(btn,&ev);
                break;
            case LeaveNotify:
                if(btn) leave_button(btn,&ev);
                break;
            case ButtonRelease:
                if(btn) btn->buttonRelease(btn->cbdata);
                break;
        }
    }

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
    res->attributes.background_pixel  = 0x000000;
    res->attributes.border_pixel      = 0xfffdd0;
    res->attributes.event_mask = ExposureMask
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

    struct mixer *m;
    char buffer[NAME_MAX];

    for(int i=0; i<nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        size_t name_len = strlen(m->ci.longname);
        char *name = malloc(sizeof(char)*name_len);
        strncpy(name,m->ci.longname,name_len);
        Window subwin = create_button(R->display, &parent, R->depth, R->visual, 
                                      *context, 0, height*i, width, &R->colormap, 
                                      0xffaa5f, 0x000000, "#00FF00", name, 
                                      name_len, NULL, font);

        /* TODO: free on close */
        Button_node *node = malloc(sizeof(Button_node));
        node->win_id = subwin;
        node->mixer_id = i;
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

/* TODO:  get rid of malloc / return char* */
char* get_mixer_info(size_t *nmixers, size_t *max_name_len)
{
    *nmixers = 0;
    *max_name_len = 0;

    if((*nmixers=mixer_get_nmixers())<0)
        errx(1,"No mixers present in system");
    
    struct mixer *m;
    char *longname = NULL;
    char buffer[NAME_MAX];

    for(size_t i=0; i<*nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        if(strlen(m->ci.longname)>*max_name_len){
            *max_name_len = MAX(*max_name_len,strlen(m->ci.longname));
            longname = malloc(sizeof(char)*(*max_name_len)+1);
            strncpy(longname,m->ci.longname,*max_name_len);
        }
         
        (void)mixer_close(m);
    }    
    return(longname);
}
        /* printf("%s\n", m->name); */
        /* printf("  - %s\n", m->ci.shortname); */
        /* printf("  - %s\n", m->ci.longname); */

#endif
