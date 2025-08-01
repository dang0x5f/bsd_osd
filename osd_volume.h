#ifndef OSD_VOLUME_H
#define OSD_VOLUME_H

/* VOLUME         MUTED */
/* ||||---------------- */

/* VOLUME            45 */
/* |||||||||----------- */

/* VOLUME            20 */
/* ████________________ */

/* VOLUME            80 */
/* ████████████████____ */

#include <err.h>
#include <time.h>
#include <mixer.h>
#include <fontconfig/fontconfig.h>

void osd_volume(char);

#endif // OSD_VOLUME_H

#ifdef OSD_VOLUME_IMPLEMENTATION

#define OSD_VOLUME_LOCK "/tmp/osd_volume.lck"
#define PX_BETWEEN 1  // 1px between top and bottom
#define SZ 20

int is_muted(void);
float getvolume(void);
void sig_handler(int);
void change_volume(char);
int create_volume_lock(void);

char *numeric_str;
char *volume_str = "VOLUME";
char *muted_str = "MUTED";

void redraw(Display*,XftDraw*,Window*, uint32_t,XftColor*,XftFont*,XftGlyphFontSpec*,size_t);
void draw_vol(Display *, XftDraw *,Window*, uint32_t,  XftColor *, XftFont *, size_t );
void draw_spec(Display *, XftDraw *,Window*,  XftColor *, XftFont *, XftGlyphFontSpec *, size_t );
XftGlyphFontSpec *create_spec(Display*,XftFont*,uint32_t,uint32_t,size_t);
XftGlyphFontSpec *spec;

float volume;
bool timeup=false;
int padding = 5;
double duration = 5.0;
time_t start_time, curr_time;

char *font_pattern = "Deja Vu Sans Mono:pixelsize=20";

void redraw(Display *display, XftDraw *draw, Window *window, uint32_t win_width, XftColor *color, XftFont *xftfont, XftGlyphFontSpec *spec, size_t sz)
{
    volume = getvolume();
    draw_vol(display, draw, window, win_width, color, xftfont, SZ);
    draw_spec(display, draw, window, color, xftfont, spec, SZ);
}

void draw_vol(Display *display, XftDraw *draw, Window *window, uint32_t win_width,  XftColor *color, XftFont *font, size_t sz)
{
    XClearArea(display,*window,win_width/2,0,0,font->ascent+padding,false);
    if(is_muted()){
        XftDrawStringUtf8(draw,color,font,(font->max_advance_width*15)+padding,font->ascent,(FcChar8*)muted_str,5);  
    }else{
        uint32_t vol = (uint32_t)(getvolume()*100);
        char volbuffer[3] = {' ',' ',' '};
        for(int i=2; i>=0; --i){
            volbuffer[i] = (vol%10)+'0';
            if(vol<10) break;
            vol /= 10;
        }
        
        XftDrawStringUtf8(draw,color,font,(font->max_advance_width*17)+padding,font->ascent,(FcChar8*)volbuffer,3);  
        
    }
}

void draw_spec(Display *display, XftDraw *draw, Window *window, XftColor *color, XftFont *font, XftGlyphFontSpec *spec, size_t sz)
{
    XClearArea(display,*window,0,font->ascent+font->descent+(padding),0,0,false);
    uint8_t nblock = (volume*100)/5;

    XftDrawGlyphFontSpec(draw,color,spec,nblock);
}

void osd_volume(char operation)
{
    change_volume(operation);

    int fd = create_volume_lock();
    if(fd==-1) return;
    close(fd);

    signal(SIGUSR1,sig_handler);

    Display *display;
    int screen_num;
    Colormap colormap;
    int depth;
    int scrn_width, scrn_height;
    int valuemask;
    Visual *visual;
    Window root;
    Window window;
    XftColor color;
    XftFont *xftfont;
    XftDraw *draw;
    XSetWindowAttributes attributes = { 
        .override_redirect=true,
        .background_pixel=0x000000,
        .border_pixel=0xfffdd0,
        .event_mask=ExposureMask|
            VisibilityChangeMask|
          SubstructureNotifyMask,
    };

    display     = XOpenDisplay(NULL);
    root        = DefaultRootWindow(display);
    screen_num  = DefaultScreen(display);
    colormap    = DefaultColormap(display,screen_num);
    xftfont     = font_setup(display,screen_num,font_pattern);
    depth       = DefaultDepth(display,screen_num);
    visual      = DefaultVisual(display,screen_num);
    scrn_width  = XDisplayWidth(display,screen_num);
    scrn_height = XDisplayHeight(display,screen_num);
    valuemask   = CWOverrideRedirect|CWBackPixel|CWEventMask|CWBorderPixel;
    XftColorAllocName(display,visual,colormap,"#00FF00",&color);

    uint32_t nlines = 2;
    uint32_t line_height = (xftfont->ascent+xftfont->descent);

    int win_width  = (xftfont->max_advance_width*SZ) + (padding*2);
    uint32_t win_height = (line_height*nlines) + (padding*2) +PX_BETWEEN;
    int x = (scrn_width*0.5)-(win_width*0.5);
    int y = (scrn_height)*0.75;
    
    window = XCreateWindow(display, root, x,y, win_width,win_height, 2, depth, 
                           CopyFromParent, visual, valuemask, &attributes);

    draw = XftDrawCreate(display,window,visual,colormap);

    XMapWindow(display,window);
    XSync(display,false);

    XftDrawStringUtf8(draw,&color,xftfont, padding, xftfont->ascent, (FcChar8*)volume_str, 6);  

    uint32_t height2 = line_height + xftfont->ascent + padding;
    XftGlyphFontSpec *spec = create_spec(display,xftfont,0x2588,height2,SZ);

    time(&start_time);
    XEvent event;
    int mute_status = is_muted();
    double difference;
    while(!timeup){
        XNextEvent(display,&event);
        switch(event.type){
            case Expose:
                redraw(display, draw, &window, win_width, &color, xftfont, spec, SZ);
                break;
            case VisibilityNotify:
                XRaiseWindow(display,window);
                break;
        }
        
        usleep(50000);
        time(&curr_time);
        difference = difftime(curr_time,start_time);

        XEvent temp;
        if(volume != getvolume()||mute_status != is_muted()){
            mute_status = is_muted();
            temp.type=Expose;
            raise(SIGUSR1);
        }
        XSendEvent(display,window,0,0,&temp);

        if(difference >= duration) timeup=true;
    }
    remove(OSD_VOLUME_LOCK);
    
    XDestroyWindow(display,window);
    XCloseDisplay(display);
}

int is_muted(void)
{
    int ismuted = 0;
    struct mixer *m;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if(!(m->dev=mixer_get_dev_byname(m,dev_name)))
        err(1,"unknown device: %s", dev_name);

    ismuted = MIX_ISMUTE(m, m->dev->devno);
    mixer_close(m);
    /* printf("%d\n",muted); */
    return(ismuted);
}

XftGlyphFontSpec *create_spec(Display *display, XftFont *font, uint32_t glyph, uint32_t height, size_t sz)
{
    XftGlyphFontSpec *spec = malloc(sizeof(XftGlyphFontSpec)*sz);

    int32_t xpos=padding, ypos=height;
    for(size_t i=0; i<sz; ++i){
        uint32_t idx = XftCharIndex(display,font,glyph);
        if(idx){
            spec[i].font = font;
            spec[i].glyph = idx;
            spec[i].x = xpos;
            spec[i].y = ypos;
            xpos += font->max_advance_width;
        }
    }
    return(spec);
}

float getvolume(void)
{
    struct mixer *m;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if(!(m->dev=mixer_get_dev_byname(m,dev_name)))
        err(1,"unknown device: %s", dev_name);

    float v = m->dev->vol.right;
    mixer_close(m);
    return(v);
}

void change_volume(char op)
{
    struct mixer *m;
    mix_volume_t vol;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if(!(m->dev=mixer_get_dev_byname(m,dev_name)))
        err(1,"unknown device: %s", dev_name);

    switch(op){
        case'!':
            /* printf("MUTE TOGGLE\n"); */
            mixer_set_mute(m,MIX_TOGGLEMUTE);
            break;
        case'+':
        case'-':
        case'u':
        case'd':
            float unit = 0.01f;
            if(op=='-'||op=='d') unit = -(unit);

            vol.right = m->dev->vol.right + unit;
            vol.left  = m->dev->vol.left  + unit;
            mixer_set_vol(m,vol);
            break;
    }

    mixer_close(m);
}

int create_volume_lock(void)
{
    int fd;
    fd=open(OSD_VOLUME_LOCK, O_WRONLY|O_CREAT|O_EXCL, S_IRWXU);
    return(fd);
}

void sig_handler(int sig)
{
    switch(sig){
        case SIGUSR1:
            time(&start_time);
            break;
    }
}

#endif // OSD_VOLUME_IMPLEMENTATION
