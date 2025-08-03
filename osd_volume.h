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

void init_window(XWindow*);
int is_muted(void);
void sig_handler(int);
float getvolume(void);
void change_volume(char);
int create_volume_lock(void);
XftGlyphFontSpec *create_glyph(XWindow*,uint32_t);
void draw(XWindow*);
void draw_vol(XWindow*);
void draw_glyph(XWindow*);
void draw_label(XWindow*,char*);

char *numeric_str;
char *volume_str = "VOLUME";
char *muted_str = "MUTED";
XftGlyphFontSpec *glyph;

float volume;
bool timeup=false;
double duration = 5.0;
time_t start_time, curr_time;

const char *font_name2 = "Deja Vu Sans Mono:pixelsize=20";

void osd_volume(char operation)
{
    change_volume(operation);

    int fd = create_volume_lock();
    if(fd==-1) return;
    close(fd);

    signal(SIGUSR1,sig_handler);

    XWindow xwin = {0};
    init_window(&xwin);

    draw_label(&xwin, "VOLUME");

    time(&start_time);
    XEvent event;
    int mute_status = is_muted();
    double difference;
    while(!timeup){
        XNextEvent(xwin.display,&event);
        switch(event.type){
            case Expose:
                draw(&xwin);
                break;
            case VisibilityNotify:
                XRaiseWindow(xwin.display,xwin.win);
                break;
        }
        
        usleep(33333);
        time(&curr_time);
        difference = difftime(curr_time,start_time);

        XEvent temp;
        if(volume != getvolume()||mute_status != is_muted()){
            mute_status = is_muted();
            temp.type=Expose;
            raise(SIGUSR1);
        }
        XSendEvent(xwin.display,xwin.win,0,0,&temp);

        if(difference >= duration) timeup=true;
    }
    remove(OSD_VOLUME_LOCK);
}

void draw(XWindow *xwin)
{
    volume = getvolume();
    draw_vol(xwin);
    draw_glyph(xwin);
}

void draw_vol(XWindow *xwin)
{
    XClearArea(xwin->display,
               xwin->win,
               xwin->width/2,
               0,
               0,
               xwin->line1_h,
               false);
    if(is_muted()){
        XftDrawStringUtf8(xwin->dc,
                          &xwin->fgcolor,
                          xwin->font,
                          (xwin->font->max_advance_width*15)+xwin->margin,
                          xwin->font->ascent,
                          (FcChar8*)muted_str,
                          5);  
    }else{
        uint32_t vol = (uint32_t)(getvolume()*100);
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

void draw_glyph(XWindow *xwin)
{
    XClearArea(xwin->display,
               xwin->win,
               0,
               xwin->line1_h,
               0,
               0,
               false);
    uint8_t nblock = (volume*100)/5;

    XftDrawGlyphFontSpec(xwin->dc,
                         &xwin->fgcolor,
                         xwin->glyph,
                         nblock);
}

void init_window(XWindow *xwin)
{
    uint8_t border_px=2;
    XftColor color;
    uint32_t screen_width=0;
    uint32_t screen_height=0;

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
    xwin->font     = font_setup(xwin->display,xwin->screen,font_name2);
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
}

void draw_label(XWindow *xwin, char *label)
{
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
