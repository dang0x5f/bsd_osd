#ifndef OSD_COMMON_H
#define OSD_COMMON_H

#define MAX(a,b) ((a)>(b)?(a):(b))

XftFont *font_setup(Display*,int,char*);

#endif // OSD_COMMON_H

#ifdef OSD_COMMON_IMPLEMENTATION

XftFont *font_setup(Display* display, int screen_num, char *font_name)
{
    XftFont *font = XftFontOpenName(display,screen_num,font_name);
    if(!font){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
    return(font);
}

#endif // OSD_COMMON_IMPLEMENTATION
