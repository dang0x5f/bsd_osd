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


void increase_volume(void);

#endif // OSD_VOLUME_H

#ifdef OSD_VOLUME_IMPLEMENTATION
char *str2 = "|||||||||-----------";
char *str = "VOLUME            45";
// char *str2 = "████████████████____";
// char block = (char)219;
// char *str = "_█";
// char *str = "_";

char *font_pattern = "Deja Vu Sans Mono:size=20";
XftFont *xftfont;
void font_setup(Display *display, int screen_num)
{
    xftfont = XftFontOpenName(display, screen_num, font_pattern);
    if(!xftfont){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
}

void increase_volume(void)
{
    int padding;

    Display *display = XOpenDisplay(NULL);
    int screen_num = DefaultScreen(display);
    font_setup(display,screen_num);

    // printf("%d\n",xftfont->max_advance_width);

    XSetWindowAttributes attributes = 
    { 
        .override_redirect=true,
        .background_pixel=0x000000,
        .event_mask=ExposureMask|SubstructureNotifyMask,
    };

    XGlyphInfo xgi;
    XftTextExtentsUtf8(display,xftfont,str,strlen(str),&xgi);
    
    int scr_width = XDisplayWidth(display,screen_num);
    int scr_height = XDisplayHeight(display,screen_num);
    // int win_width = xftfont->max_advance_width * strlen(str);
    int win_width = xgi.xOff;
    int win_height = xftfont->height*2;

    int x = (scr_width/2) - (win_width/2);
    int y = (scr_height/4)*3;

    printf("%d , %lu\n", xftfont->max_advance_width, strlen(str));
    printf("%dx%d, %d\n", win_width, win_height, xftfont->ascent);
    
    Window window = XCreateWindow(display, DefaultRootWindow(display), 
                    x,y, win_width,win_height, 0, DefaultDepth(display,screen_num),  
                    CopyFromParent, DefaultVisual(display,screen_num), 
                    CWOverrideRedirect|CWBackPixel|CWEventMask, &attributes);

    XftDraw *draw = XftDrawCreate(display,window,
                    DefaultVisual(display,screen_num),
                    DefaultColormap(display,screen_num));

    XftColor color;
    XftColorAllocName(display,DefaultVisual(display,screen_num),
            DefaultColormap(display,screen_num),"#00FF00",&color);
    XMapWindow(display,window);
    XSync(display,false);


    XEvent event;
    while(1){
        XNextEvent(display,&event);
        switch(event.type){
            case Expose:
                XftDrawStringUtf8(draw,&color,xftfont,0,xftfont->ascent,
                                  (FcChar8*)str,strlen(str));
                XftDrawStringUtf8(draw,&color,xftfont,0,xftfont->ascent*2,
                                  (FcChar8*)str2,strlen(str2));
                break;
        }
    }

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
