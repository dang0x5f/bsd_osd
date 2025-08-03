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

#define BLACK1 0x000000
#define WHITE1 0xfffdd0
#define GREEN "#00FF00"
#define OSD_VOLUME_LOCK "/tmp/osd_volume.lck"
#define PX_BETWEEN 1  
#define MIXER_MUTED_FLAG 0
#define SZ 20

typedef struct {
    uint32_t depth;
    uint32_t width;
    uint32_t height;
    uint32_t attrmask;
    int32_t xpos;
    int32_t ypos;
    Display* display;
    Visual *visual;
    XftFont *font;
    XftDraw *dc;
    Window win;
    Window parent;
    Colormap cmap;
    XSetWindowAttributes xswa;
    uint8_t screen;
    uint8_t margin; 
    XftColor fgcolor;
    XftGlyphFontSpec *glyph; 
    uint32_t line1_h;
    uint32_t line2_h;
} XWindow;

typedef struct {
    time_t start_time; 
    time_t curr_time;
    double difference;
    double duration; // = 5.0;
} TimerState;

typedef struct {
    int mute_flag;
    float volumef;
    int volumei;
} MixerState;

void init_window(XWindow*);
int is_muted(void);
void sig_handler(int);
float get_volume(void);
void change_volume(char);
int create_volume_lock(void);
XftGlyphFontSpec *create_glyph(XWindow*,uint32_t);
void draw(XWindow*,MixerState*);
void draw_vol(XWindow*,MixerState*);
void draw_glyph(XWindow*,MixerState*);
void draw_label(XWindow*);
bool check_timer(XWindow*,TimerState*,MixerState*,XEvent*);

void init_timer(TimerState *ts)
{
    time(&ts->start_time);
    ts->duration = 5.0;
}

void init_mixer(MixerState *ms)
{
    ms->mute_flag = is_muted();
}

void osd_volume(char operation)
{
    change_volume(operation);

    int fd = create_volume_lock();
    if(fd==-1) return;
    close(fd);

    bool running  = true;
    XEvent event  = {0};
    XWindow xwin  = {0};
    TimerState ts = {0};
    MixerState ms = {0};

    init_window(&xwin);
    init_timer(&ts);
    init_mixer(&ms);

    while(running){
        XNextEvent(xwin.display,&event);
        switch(event.type){
            case Expose:
                draw(&xwin,&ms);
                break;
            case VisibilityNotify:
                XRaiseWindow(xwin.display,xwin.win);
                break;
        }
        running=check_timer(&xwin,&ts,&ms,&event);
    }
    remove(OSD_VOLUME_LOCK);
}

bool check_timer(XWindow *xwin, TimerState *ts, MixerState *ms, XEvent *event)
{
    usleep(33333);

    float curr_volumef = get_volume();
    int curr_mute_flag = is_muted();

    time(&ts->curr_time);
    ts->difference = difftime(ts->curr_time,ts->start_time);

    if(ms->volumef != curr_volumef||ms->mute_flag != curr_mute_flag){
        ms->mute_flag = curr_mute_flag;
        event->type=Expose;
        time(&ts->start_time);
    }
    XSendEvent(xwin->display,xwin->win,0,0,event);

    return( ts->difference >= ts->duration?false:true );
}

void draw(XWindow *xwin, MixerState *ms)
{
    ms->volumef = get_volume();
    draw_vol(xwin,ms);
    draw_glyph(xwin,ms);
}

void draw_vol(XWindow *xwin, MixerState *ms)
{
    static const char *muted_str = "MUTED";
    XClearArea(xwin->display,
               xwin->win,
               xwin->width/2,
               0,
               0,
               xwin->line1_h,
               false);
    if(is_muted()){
        size_t length = strlen(muted_str);
        XftDrawStringUtf8(xwin->dc,
                          &xwin->fgcolor,
                          xwin->font,
                          (xwin->font->max_advance_width*15)+xwin->margin,
                          xwin->font->ascent,
                          (FcChar8*)muted_str,
                          length);  
    }else{
        ms->volumei = (uint32_t)(ms->volumef*100);
        uint32_t vol = ms->volumei;
        char volbuffer[3] = {' ',' ',' '};
        for(int i=2; i>=0; --i){
            volbuffer[i] = (vol%10)+'0';
            if(vol<10) break;
            vol /= 10;
        }
        
        XftDrawStringUtf8(xwin->dc,
                          &xwin->fgcolor,
                          xwin->font,
                          (xwin->font->max_advance_width*17)+xwin->margin,
                          xwin->font->ascent,
                          (FcChar8*)volbuffer,
                          3);  
        
    }
}

void draw_glyph(XWindow *xwin, MixerState *ms)
{
    XClearArea(xwin->display,
               xwin->win,
               0,
               xwin->line1_h,
               0,
               0,
               false);
    uint8_t nblock = (ms->volumef*100)/5;

    XftDrawGlyphFontSpec(xwin->dc,
                         &xwin->fgcolor,
                         xwin->glyph,
                         nblock);
}

void init_window(XWindow *xwin)
{
    XftColor color;
    uint8_t border_px=2;
    uint32_t screen_width=0;
    uint32_t screen_height=0;
    char *font_name = "Deja Vu Sans Mono:pixelsize=20";

    xwin->display  = XOpenDisplay(NULL);
    xwin->screen   = DefaultScreen(xwin->display);
    xwin->parent   = DefaultRootWindow(xwin->display);
    xwin->depth    = DefaultDepth(xwin->display,xwin->screen);
    xwin->cmap     = DefaultColormap(xwin->display,xwin->screen);
    xwin->visual   = DefaultVisual(xwin->display,xwin->screen);
    xwin->attrmask = CWOverrideRedirect|
                          CWBorderPixel|
                            CWBackPixel|
                            CWEventMask;
    xwin->xswa .override_redirect=true;
    xwin->xswa.background_pixel=BLACK1;
    xwin->xswa.border_pixel=WHITE1;
    xwin->xswa.event_mask=ExposureMask| 
                  VisibilityChangeMask| 
                SubstructureNotifyMask;
    screen_width   = XDisplayWidth(xwin->display,xwin->screen);
    screen_height  = XDisplayHeight(xwin->display,xwin->screen);
    xwin->font     = font_setup(xwin->display,xwin->screen,font_name);
    XftColorAllocName(xwin->display,xwin->visual,xwin->cmap,GREEN,&color);
    xwin->fgcolor  = color;
    xwin->margin   = 5;
    xwin->width    = (xwin->font->max_advance_width*SZ)+
                     (xwin->margin*2); 
    xwin->height   = (xwin->font->ascent*2)+
                     (xwin->font->descent)+
                     (xwin->margin*2)+
                     PX_BETWEEN;
    xwin->xpos     = (screen_width*0.5)-(xwin->width*0.5);
    xwin->ypos     = (screen_height*0.75);

    xwin->win      = XCreateWindow(xwin->display, 
                                   xwin->parent, 
                                   xwin->xpos,
                                   xwin->ypos, 
                                   xwin->width,
                                   xwin->height, 
                                   border_px, 
                                   xwin->depth, 
                                   CopyFromParent, 
                                   xwin->visual, 
                                   xwin->attrmask, 
                                   &xwin->xswa);

    xwin->dc       = XftDrawCreate(xwin->display,
                                   xwin->win,
                                   xwin->visual,
                                   xwin->cmap);

    xwin->line1_h  = (xwin->font->ascent);
    xwin->line2_h  = (xwin->line1_h*2)+xwin->margin;
    xwin->glyph    = create_glyph(xwin,0x2588);

    XMapWindow(xwin->display,xwin->win);
    XSync(xwin->display,false);

    draw_label(xwin);
}

void draw_label(XWindow *xwin)
{
    char *label = "VOLUME";
    size_t length = strlen(label);
    XftDrawStringUtf8(xwin->dc,
                      &xwin->fgcolor,
                      xwin->font, 
                      xwin->margin, 
                      xwin->line1_h, 
                      (FcChar8*)label, 
                      length);  
}

int is_muted(void)
{
    int ismuted = MIXER_MUTED_FLAG;
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

    return(ismuted);
}

XftGlyphFontSpec *create_glyph(XWindow *xwin, uint32_t glyph)
{
    XftGlyphFontSpec *spec = malloc(sizeof(XftGlyphFontSpec)*SZ);

    int32_t xpos=xwin->margin, ypos=xwin->line2_h;
    for(size_t i=0; i<SZ; ++i){
        uint32_t idx = XftCharIndex(xwin->display,xwin->font,glyph);
        if(idx){
            spec[i].font = xwin->font;
            spec[i].glyph = idx;
            spec[i].x = xpos;
            spec[i].y = ypos;
            xpos += xwin->font->max_advance_width;
        }
    }
    return(spec);
}

float get_volume(void)
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

#endif // OSD_VOLUME_IMPLEMENTATION
