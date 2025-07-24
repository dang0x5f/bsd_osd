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

WinResources *init_resources(void);    // Setup window essentials
Button_List create_buttonlist(WinResources*,Window,XContext*,int,int,int,XftFont*);   
char* get_mixer_info(size_t*,size_t*);
int get_defaultunit(void);     
void set_defaultunit(void*);

char *font_name = "Deja Vu Sans Mono:pixelsize=16";


int osd_outmixer(void)
{
    XContext context = XUniqueContext();

    WinResources *R = init_resources();

    XftFont *font = font_setup(R->display,R->screen_num,font_name);

    size_t nmixers, max_name_len;
    char *name = get_mixer_info(&nmixers,&max_name_len);
    printf("%zu\n",nmixers);

    /* int height = (nmixers*2)*font->height; */


    int ypadding = 3;
    int padding = 5;

    int width = font->max_advance_width * max_name_len+(BORDER_PIXEL*2) + (padding*2);
    int height = (font->ascent+font->descent+(BORDER_PIXEL*2)+(BORDER_PIXEL*2))*(nmixers-1);

    height += (ypadding*(nmixers-2));

    int xpos = DisplayWidth(R->display,R->screen_num);
    int ypos = DisplayHeight(R->display,R->screen_num);
    ypos = (ypos/2)-(height/2);
    
    Window root = DefaultRootWindow(R->display);
    /* TODO: make less messy, and add remarks why */
    // width  + (BORDER*2)                  -  account for button border
    // height + (BORDER*2) + (BORDER*2)     -  account for button border .. 2 times works?
    Window window = XCreateWindow(R->display,root,XPOS,ypos,width,height,
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

    printf("%d\n",get_defaultunit());

    XGrabKeyboard(R->display,window,true,GrabModeAsync,GrabModeAsync,CurrentTime);
    
    bool running=true;
    XEvent ev;
    while(running){
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
                if(keysym==XK_Escape || keysym==XK_q) running=false;
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

    struct mixer *m;
    char buffer[NAME_MAX];

    for(int i=0; i<nmixers; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        size_t name_len = strlen(m->ci.longname);
        char *name = malloc(sizeof(char)*name_len);
        strncpy(name,m->ci.longname,name_len);
        Window subwin = create_button(R->display, &parent, R->depth, R->visual, 
                                      *context, 0, height*i+(i*3), width, &R->colormap, 
                                      0x333333, 0xbbbbbb, "#000000", name, 
                                      name_len, set_defaultunit, font);

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

int get_defaultunit(void)
{
    // TODO: input should either be initialized or checked before return
    int input;
    size_t ilen = sizeof(input);
    // TODO: use errno
    if(sysctlbyname("hw.snd.default_unit",&input,&ilen,NULL,0)==-1){
        perror("sysctlbyname() error\n");
        exit(EXIT_FAILURE);
    }
    return(input);
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

        /* printf("%s\n", m->name); */
        /* printf("  - %s\n", m->ci.shortname); */
        /* printf("  - %s\n", m->ci.longname); */

#endif
