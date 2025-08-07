#ifndef OSD_PROGLIST_H
#define OSD_PROGLIST_H

void osd_proglist(void);

#endif

#ifdef OSD_PROGLIST_IMPL

void osd_proglist(void)
{
    Display *display = XOpenDisplay(NULL); 
    int screen = DefaultScreen(display);
    
    uint32_t nchildren;
    Window root_ret, parent_ret, *children_ret;
    Window root = DefaultRootWindow(display);

    XQueryTree(display,
               root,
               &root_ret,
               &parent_ret,
               &children_ret,
               &nchildren);

    XTextProperty prop;

    Atom actual_type;
    int32_t actual_format;
    uint64_t nitems, bytes_after;
    unsigned char *prop_ret = NULL; 

    Atom wm_state_atom = XInternAtom(display,"WM_STATE",false);

    for(size_t i=0; i<nchildren; ++i){

        XWindowAttributes attr;
        XGetWindowAttributes(display,children_ret[i],&attr);
        if(attr.map_state == IsViewable){
            XGetWMName(display,children_ret[i],&prop);
            printf("(%d) %s\n", children_ret[i],prop.value);
            continue;
        }

        Status status = XGetWindowProperty(display,
                                           children_ret[i],
                                           wm_state_atom,
                                           0, (~0L),
                                           false,
                                           AnyPropertyType,
                                           &actual_type, &actual_format,
                                           &nitems, &bytes_after,
                                           &prop_ret);

        if(status == Success && prop_ret != NULL){
            int64_t *state_data = (int64_t*)prop_ret;
            int64_t state = state_data[0];

            if(state != WithdrawnState){
                XGetWMName(display,children_ret[i],&prop);
                printf("(%d) %s\n", children_ret[i],prop.value);
                continue;
            }
        }

    }
               
}

void osd_proglist2(void)
{
    Display *display = XOpenDisplay(NULL); 
    int screen = DefaultScreen(display);

    XSetWindowAttributes attr = {
        .background_pixel=0xffff00,
        .border_pixel=0x007700,
        .event_mask = ExposureMask|
              VisibilityChangeMask|
            SubstructureNotifyMask,
    };
    int mask=CWBorderPixel|
               CWBackPixel|
               CWEventMask;
    
    
    char *prog_name = "prog_list";
    XTextProperty prop;
    XTextProperty prop2;

    Window win = XCreateWindow(display, 
                               DefaultRootWindow(display),
                               0,0,
                               400,400,
                               2,
                               DefaultDepth(display,screen),
                               CopyFromParent,
                               DefaultVisual(display,screen),
                               mask,
                               &attr);
                               


    XStringListToTextProperty(&prog_name,1,&prop);
    XSetWMName(display,win,&prop);
    
    XGetWMName(display,win,&prop2);

    char **name;
    int cnt = 1;
    XTextPropertyToStringList(&prop2,&name,&cnt);
    printf("%s\n", *name);


    /* Atom atom = XInternAtom(display, "WM_DELETE_WINDOW", false); */

    /* if(atom == None){ */
    /*     fprintf(stderr,"Failed to intern WM_DELETE_WINDOW atom\n"); */
    /* } else { */
    /*     printf("WM_DELETE_WINDOW Atom: %lu\n", (uint32_t)atom); */
    /*     printf("String: %s\n", XGetAtomName(display, atom)); */
    /* } */


    /* printf("osd_proglist()\n"); */

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
