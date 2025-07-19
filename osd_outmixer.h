#ifndef OSD_OUTMIXER_H
#define OSD_OUTMIXER_H

#include <sys/sysctl.h>
#include <mixer.h>

int osd_outmixer(void);         // Driver function
void *create_mixerlist(void);   // Retrieve list of mixer devices
void get_defaultunit(void);     
void set_defaultunit(void);
void create_buttonlist(void);   

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

char *font_name = "Deja Vu Sans Mono:pixelsize=20";
XftFont *font_setup2(Display *display, int screen_num)
{
    XftFont *xftfont = XftFontOpenName(display,screen_num,font_name);
    if(!xftfont){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
    return(xftfont);
}

int osd_outmixer(void)
{
    WinResources *R = init_resources();

    XftFont *font = font_setup2(R->display,R->screen_num);

    int nmixers;
    if((nmixers=mixer_get_nmixers())<0)
        errx(1,"No mixers present in system");

    int height = (nmixers*2)*font->height;
    
    Window root = DefaultRootWindow(R->display);
    Window window = XCreateWindow(R->display,root,XPOS,YPOS,WIDTH,height,
                                  BORDER_PIXEL,R->depth,CopyFromParent,
                                  R->visual,R->valuemask,&R->attributes);

    XMapWindow(R->display,window);
    XSync(R->display,false);
    
    while(1){

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
    res->attributes.border_pixel      =0xfffdd0;
    res->attributes.event_mask = ExposureMask
                               | VisibilityChangeMask
                               | SubstructureNotifyMask;
    res->valuemask = CWOverrideRedirect
                   | CWBackPixel
                   | CWEventMask
                   | CWBorderPixel;

    return(res);
}

void create_buttonlist(void)
{
    struct mixer *m;
    /* struct mix_dev *md; */
    char buffer[NAME_MAX];
    int n;
    
    if((n=mixer_get_nmixers())<0)
        errx(1,"No mixers present in system");
    for(int i=0; i<n; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        printf("%s\n", m->name);
        printf("  - %s\n", m->ci.shortname);
        printf("  - %s\n", m->ci.longname);

        (void)mixer_close(m);
    }    
}

#endif
