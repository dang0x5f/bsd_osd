#ifndef OSD_PROGLIST_H
#define OSD_PROGLIST_H

void osd_proglist(void);

#endif

#ifdef OSD_PROGLIST_IMPL

void osd_proglist(void)
{
    
    printf("osd_proglist()\n");

}

              // ==== 1 ==== //

/* void enumerate_windows(Display *display, Window window_id, int indent) */
/* { */
/*     Window root_ret, parent_ret, *child_ret; */
/*     char *window_name; */
/*     uint32_t nchildren; */

/*     for(int i=0; i<indent; ++i) printf("  "); */

/*     XFetchName(display,window_id,&window_name); */
/*     printf("Window ID: 0x%lx ( %s )\n",window_id, window_name); */

/*     if(XQueryTree(display,window_id,&root_ret,&parent_ret,&child_ret,&nchildren)){ */
/*         for(uint32_t i=0; i<nchildren; ++i){ */
/*             enumerate_windows(display,child_ret[i],indent+1); */
/*         } */

/*         if(child_ret != NULL) XFree(child_ret); */
/*     } */
/* } */

/* void osd_proglist(void) */
/* { */
/*     Display *dpy; */
/*     Window root; */

/*     dpy = XOpenDisplay(NULL); */
/*     root = DefaultRootWindow(dpy); */

/*     enumerate_windows(dpy,root,0); */

/*     XCloseDisplay(dpy); */
/* } */

              // ==== 2 ==== //

/* Window *winlist (Display *disp, unsigned long *len); */
/* char *winame (Display *disp, Window win); */ 
 
/* void osd_proglist(void) { */
/*     int i; */
/*     unsigned long len; */
/*     Display *disp = XOpenDisplay(NULL); */
/*     Window *list; */
/*     char *name; */
 
/*     if (!disp) { */
/*         puts("no display!"); */
/*     } */
 
/*     list = (Window*)winlist(disp,&len); */
 
/*     for (i=0;i<(int)len;i++) { */
/*         name = winame(disp,list[i]); */
/*         printf("-->%s<--\n",name); */
/*         free(name); */
/*     } */
 
/*     XFree(list); */
 
/*     XCloseDisplay(disp); */
/* } */
 
 
/* Window *winlist (Display *disp, unsigned long *len) { */
/*     Atom prop = XInternAtom(disp,"_NET_CLIENT_LIST",False), type; */
/*     int form; */
/*     unsigned long remain; */
/*     unsigned char *list; */
 
/*     errno = 0; */
/*     if (XGetWindowProperty(disp,XDefaultRootWindow(disp),prop,0,1024,False,XA_WINDOW, */
/*                 &type,&form,len,&remain,&list) != Success) { */
/*         perror("winlist() -- GetWinProp"); */
/*         return 0; */
/*     } */
     
/*     return (Window*)list; */
/* } */
 
 
/* char *winame (Display *disp, Window win) { */
/*     Atom prop = XInternAtom(disp,"WM_NAME",False), type; */
/*     int form; */
/*     unsigned long remain, len; */
/*     unsigned char *list; */
 
/*     errno = 0; */
/*     if (XGetWindowProperty(disp,win,prop,0,1024,False,XA_STRING, */
/*                 &type,&form,&len,&remain,&list) != Success) { */
/*         perror("winlist() -- GetWinProp"); */
/*         return NULL; */
/*     } */
 
/*     return (char*)list; */
/* } */

#endif 
