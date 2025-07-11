#ifndef OSD_VOLUME_H
#define OSD_VOLUME_H

/* VOLUME            20 */
/* ||||---------------- */

/* VOLUME            45 */
/* |||||||||----------- */

/* VOLUME            20 */
/* ████________________ */

/* VOLUME            80 */
/* ████████████████____ */

#include <err.h>
#include <mixer.h>
#include <fontconfig/fontconfig.h>


void increase_volume(void);
float getvolume(void);
void getspec1(float);

#endif // OSD_VOLUME_H

#ifdef OSD_VOLUME_IMPLEMENTATION

#define SZ 20

char *font_pattern = "Deja Vu Sans Mono:pixelsize=20";
char *str2 = "|||||||||-----------";
char *def_line1 = "VOLUME              ";

XftFont *font_setup(Display *display, int screen_num)
{
    XftFont *xftfont = XftFontOpenName(display, screen_num, font_pattern);
    if(!xftfont){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
    return(xftfont);
}

void increase_volume(void)
{
    Display *display;
    int screen_num;
    int padding = 0;
    Colormap colormap;
    int depth;
    int scrn_width, scrn_height;
    int valuemask;
    Visual *visual;
    Window root;
    Window window;
    XftColor color;
    XftFont *xftfont;
    XGlyphInfo xgi;
    XftDraw *draw;
    XSetWindowAttributes attributes = { 
        .override_redirect=true,
        .background_pixel=0x000000,
        .event_mask=ExposureMask|SubstructureNotifyMask,
    };

    display     = XOpenDisplay(NULL);
    root        = DefaultRootWindow(display);
    screen_num  = DefaultScreen(display);
    colormap    = DefaultColormap(display,screen_num);
    xftfont     = font_setup(display,screen_num);
    depth       = DefaultDepth(display,screen_num);
    visual      = DefaultVisual(display,screen_num);
    scrn_width  = XDisplayWidth(display,screen_num);
    scrn_height = XDisplayHeight(display,screen_num);
    valuemask   = CWOverrideRedirect|CWBackPixel|CWEventMask;
    XftColorAllocName(display,visual,colormap,"#00FF00",&color);

    int win_width  = xftfont->max_advance_width*SZ;
    int win_height = xftfont->height*2;
    int x = (scrn_width*0.5)-(win_width*0.5);
    int y = (scrn_height)*0.75;
    
    window = XCreateWindow(display, root, x,y, win_width,win_height, 0, depth, 
                           CopyFromParent, visual, valuemask, &attributes);

    draw = XftDrawCreate(display,window,visual,colormap);

    XMapWindow(display,window);
    XSync(display,false);

    XEvent event;
    while(1){
        XNextEvent(display,&event);
        switch(event.type){
            case Expose:
                int lineno = 1;
                float volume = getvolume();
                // printf("%0.2f\n",volume);
                XftGlyphFontSpec *spec1 = getspec1(volume);
                break;
        }
    }

}

void getspec1(float)
{

}

float getvolume(void)
{
    struct mixer *m;
    mix_volume_t vol;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if((m->dev=mixer_get_dev_byname(m,dev_name))<0)
        err(1,"unknown device: %s", dev_name);

    // printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 

    // vol.left = m->dev->vol.left + 0.01;
    // vol.right = m->dev->vol.right + 0.01;
    // mixer_set_vol(m,vol);
    // if(mixer_set_vol(m,vol) < 0)
    //     warn("cannot change volume");
    
    // printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 
    return(m->dev->vol.right);
}

void increase_volume1(void)
{
    printf("Hello, World!\n");
    struct mixer *m;
    mix_volume_t vol;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if((m->dev=mixer_get_dev_byname(m,dev_name))<0)
        err(1,"unknown device: %s", dev_name);

    printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 

    vol.left = m->dev->vol.left + 0.01;
    vol.right = m->dev->vol.right + 0.01;
    mixer_set_vol(m,vol);
    // if(mixer_set_vol(m,vol) < 0)
    //     warn("cannot change volume");
    
    printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 
}


#endif // OSD_VOLUME_IMPLEMENTATION
