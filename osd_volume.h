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
#include <time.h>
#include <mixer.h>
#include <fontconfig/fontconfig.h>

void osd_volume(char);

#endif // OSD_VOLUME_H

#ifdef OSD_VOLUME_IMPLEMENTATION

#define OSD_VOLUME_LOCK "/tmp/osd_volume.lck"
#define PX_BETWEEN 1  // 1px between top and bottom
#define SZ 20

float getvolume(void);
void sig_handler(int);
void change_volume(char);
int create_volume_lock(void);
XftGlyphFontSpec *getspec1(Display*, XftFont*, float);
XftGlyphFontSpec *getspec2(Display*, XftFont*, float);


bool timeup=false;
int padding = 10;
double duration = 5.0;
time_t start_time, curr_time;
char *def_line1 = "VOLUME              ";
char *font_pattern = "Deja Vu Sans Mono:pixelsize=20";


XftFont *font_setup(Display *display, int screen_num)
{
    XftFont *xftfont = XftFontOpenName(display, screen_num, font_pattern);
    if(!xftfont){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
    return(xftfont);
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
    // XGlyphInfo xgi;
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
    xftfont     = font_setup(display,screen_num);
    depth       = DefaultDepth(display,screen_num);
    visual      = DefaultVisual(display,screen_num);
    scrn_width  = XDisplayWidth(display,screen_num);
    scrn_height = XDisplayHeight(display,screen_num);
    valuemask   = CWOverrideRedirect|CWBackPixel|CWEventMask|CWBorderPixel;
    XftColorAllocName(display,visual,colormap,"#00FF00",&color);

    // int win_width  = xftfont->max_advance_width*SZ;
    int win_width  = (xftfont->max_advance_width*SZ) + (padding*2);
    // int win_height = xftfont->height*2;
    // int win_height = (xftfont->height*2) + (xftfont->descent*2);
    // int win_height = xftfont->ascent*2;
    int win_height = (xftfont->ascent*2) + (padding*2) +PX_BETWEEN;
    int x = (scrn_width*0.5)-(win_width*0.5);
    int y = (scrn_height)*0.75;
    
    window = XCreateWindow(display, root, x,y, win_width,win_height, 2, depth, 
                           CopyFromParent, visual, valuemask, &attributes);

    draw = XftDrawCreate(display,window,visual,colormap);

    XMapWindow(display,window);
    XSync(display,false);

    time(&start_time);
    XEvent event;
    float volume;
    double difference;
    while(!timeup){
        XNextEvent(display,&event);
        switch(event.type){
            case Expose:
                XClearWindow(display,window);
                // int lineno = 1;
                volume = getvolume();
                XftGlyphFontSpec *spec1 = getspec1(display,xftfont,volume);
                XftGlyphFontSpec *spec2 = getspec2(display,xftfont,volume);
                XftDrawGlyphFontSpec(draw,&color,spec1,SZ);
                XftDrawGlyphFontSpec(draw,&color,spec2,SZ);
                break;
            case VisibilityNotify:
                XRaiseWindow(display,window);
                break;
        }
        
        usleep(10000);
        time(&curr_time);
        difference = difftime(curr_time,start_time);

        XEvent temp;
        if(volume != getvolume()){
            temp.type=Expose;
            raise(SIGUSR1);
        }else{
            temp.type=VisibilityNotify;
        }
        XSendEvent(display,window,0,0,&temp);
        XFlush(display);

        if(difference >= duration) timeup=true;
    }
    remove(OSD_VOLUME_LOCK);
    // TODO: free allocation
}

XftGlyphFontSpec *getspec1(Display *display, XftFont *font, float volume)
{
    int vol = volume*100;
    XftGlyphFontSpec *spec = malloc(sizeof(XftGlyphFontSpec)*SZ);
    // TODO: add malloc ptr check

    int xpos=padding, ypos=(font->ascent-font->descent) +padding;
    for(int i=0; i<SZ; ++i){
        uint32_t glyph = (uint32_t)0x00 << 24 |
                         (uint32_t)0x00 << 16 |
                         (uint32_t)0x00 <<  8 |
                 (uint32_t)def_line1[i] <<  0 ;
        
        uint32_t idx = XftCharIndex(display,font,glyph);
        if(idx){
            spec[i].font = font;
            spec[i].glyph = idx;
            spec[i].x = xpos;
            spec[i].y = ypos;
            xpos += font->max_advance_width;
        }
    }

    for(int i=SZ-1; ; --i){
        char c = (vol%10)+'0';
        uint32_t glyph = (uint32_t)0x00 << 24 |
                         (uint32_t)0x00 << 16 |
                         (uint32_t)0x00 <<  8 |
                         (uint32_t)   c <<  0 ;
        uint32_t idx = XftCharIndex(display,font,glyph);
        spec[i].glyph = idx;
        if(vol<10) break;
        vol /= 10;
    }
    return(spec);
}

XftGlyphFontSpec *getspec2(Display *display, XftFont *font, float volume)
{
    int vol = volume*100;
    XftGlyphFontSpec *spec = malloc(sizeof(XftGlyphFontSpec)*SZ);
    // TODO: add malloc ptr check

    int xpos=padding, ypos=(font->ascent*2-font->descent) +padding +PX_BETWEEN;
    int nblock = vol/5;
    int i=0;
    for(;i<nblock;++i){
        uint32_t glyph = (uint32_t)0x00 << 24 |
                         (uint32_t)0x00 << 16 |
                         (uint32_t)0x25 <<  8 |
                         (uint32_t)0x88 <<  0 ;
        
        uint32_t idx = XftCharIndex(display,font,glyph);
        if(idx){
            spec[i].font = font;
            spec[i].glyph = idx;
            spec[i].x = xpos;
            spec[i].y = ypos;
            xpos += font->max_advance_width;
        }
    }
    for(;i<SZ;++i){
        uint32_t glyph = (uint32_t)0x00 << 24 |
                         (uint32_t)0x00 << 16 |
                         (uint32_t)0x00 <<  8 |
                         (uint32_t)0x5f <<  0 ;
        
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

    return(m->dev->vol.right);
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

    float unit = 0.01f;
    if(op=='-'||op=='d') unit = -(unit);

    vol.left = m->dev->vol.left + unit;
    vol.right = m->dev->vol.right + unit;
    mixer_set_vol(m,vol);
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
