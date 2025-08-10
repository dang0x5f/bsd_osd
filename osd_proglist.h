#ifndef OSD_PROGLIST_H
#define OSD_PROGLIST_H

void osd_proglist(void);

#endif

#ifdef OSD_PROGLIST_IMPL


typedef struct {
    Window winid;
    Window root;
    Display *display;
    int screen;
    XftFont *font;
    int depth;
    Visual *visual;
    Colormap colormap;
    size_t width,height;
    size_t screen_width,screen_height;
} XWindow_main;

typedef struct {
    Button *button;
    char *name;
    size_t namelen;
    uint8_t workspace;
} XWindow_app;

/* typedef struct { */
/*     bool todo; */
/* } NodeData; */

/* typedef struct appnode_t{ */
/*     Window win_id; */
/*     /1* uint8_t mixer_id; *1/ */
/*     Button *btn; */
/*     struct node_t *next; */
/*     struct node_t *prev; */
/* } App_Node; */

/* typedef struct { */
/*     App_Node *first; */
/*     App_Node *end; */
/*     /1* uint8_t default_mixer; *1/ */
/*     App_Node *selected; */
/*     size_t length; */
/* } App_List; */

/* create_button(Display *display, Window *parent, Window *child, int depth, */ 
/*               Visual *visual, XContext context, int x, int y, int width, int height, */
/*               Colormap *colormap, int border_px, int border, int background, char *foreground, */ 
/*               char *label, size_t label_len, Callback cb_func, XftFont *xftfont) */

// #ff4040

void osd_proglist(void)
{
    XWindowChanges changes;

    XWindow_main xwmain = {0};
    LinkList list = {0};

    xwmain.display = XOpenDisplay(NULL); 
    xwmain.screen = DefaultScreen(xwmain.display);
    xwmain.root = DefaultRootWindow(xwmain.display);
    xwmain.font = font_setup(xwmain.display, xwmain.screen,
                             "Deja Vu Sans Mono:pixelsize=16");
    xwmain.depth = DefaultDepth(xwmain.display, xwmain.screen);
    xwmain.visual = DefaultVisual(xwmain.display, xwmain.screen);
    xwmain.colormap = DefaultColormap(xwmain.display, xwmain.screen);
    xwmain.screen_width = DisplayWidth(xwmain.display,xwmain.screen);
    xwmain.screen_height = DisplayHeight(xwmain.display,xwmain.screen);

    int x=0, y=0;
    int border_width = 2;
    int valuemask=CWEventMask|CWBackPixel|CWOverrideRedirect;
    XSetWindowAttributes attributes = {
        .override_redirect = true,
        .background_pixel = 0xfffdd0,
        .event_mask = ExposureMask|
                      KeyPressMask|
                      SubstructureNotifyMask,
    };
    
    /* xwmain.width = xwmain.screen_width-100; */
    /* xwmain.height = xwmain.screen_height-100; */
    xwmain.winid = XCreateWindow(xwmain.display, 
                                 xwmain.root, 
                                 x, y, 
                                 xwmain.screen_width, 
                                 xwmain.screen_height, 
                                 border_width, 
                                 xwmain.depth, 
                                 InputOutput, 
                                 xwmain.visual, 
                                 valuemask, 
                                 &attributes);

    /* XGrabKeyboard(xwmain.display,xwmain.winid,true,GrabModeAsync,GrabModeAsync,CurrentTime); */
    /* XEvent event; */
    /* while(1) { */
    /*     XNextEvent(xwmain.display,&event); */
    /*     switch(event.type){ */
    /*         case Expose: */
    /*             break; */
    /*         case KeyPress: */
    /*             printf("%d\n",event.xkey.keycode); */
    /*         default: */
    /*             break; */
    /*     } */
    /*     if(event.xkey.keycode == 9||event.xkey.keycode == 53){ */
    /*         break; */
    /*     } */
    /* } */
    /* XUngrabKeyboard(xwmain.display,CurrentTime); */
    
    uint32_t nchildren;
    Window root_ret, parent_ret, *children_ret;
    XQueryTree(xwmain.display,
               xwmain.root,
               &root_ret,
               &parent_ret,
               &children_ret,
               &nchildren);

    XTextProperty prop;

    Atom actual_type;
    int32_t actual_format;
    uint64_t nitems, bytes_after;
    unsigned char *prop_ret = NULL; 

    XClassHint class_hint;
    Atom wm_state_atom = XInternAtom(xwmain.display, "WM_STATE",false);

    for(size_t i=0; i<nchildren; ++i){

        XWindowAttributes attr;
        XGetWindowAttributes(xwmain.display,children_ret[i],&attr);
        if(!attr.override_redirect && attr.map_state == IsViewable){
            XGetClassHint(xwmain.display,children_ret[i],&class_hint);
            /* printf("res_name: %s\n",class_hint.res_name); */
            printf("res_class: %s\n",class_hint.res_class);
            /* XGetWMName(xwmain.display,children_ret[i],&prop); */
            /* printf("(%d) %s\n", children_ret[i],prop.value); */
            /* list.length += 1; */
            xwmain.height += 1;
            if(strlen(class_hint.res_class) > xwmain.width)
                xwmain.width = strlen(class_hint.res_class);
            continue;
        }

        Status status = XGetWindowProperty(xwmain.display,
                                           children_ret[i],
                                           wm_state_atom,
                                           0, sizeof(int64_t),
                                           false,
                                           AnyPropertyType,
                                           &actual_type, &actual_format,
                                           &nitems, &bytes_after,
                                           &prop_ret);

        if(status == Success && prop_ret != NULL){
            int64_t *state_data = (int64_t*)prop_ret;
            int64_t state = state_data[0];

            if(state != WithdrawnState){
                XGetClassHint(xwmain.display,children_ret[i],&class_hint);
                /* printf("res_name: %s\n",class_hint.res_name); */
                printf("res_class: %s\n",class_hint.res_class);
                /* XGetWMName(xwmain.display,children_ret[i],&prop); */
                /* printf("(%d) %s\n", children_ret[i],prop.value); */
                /* list.length += 1; */
                xwmain.height += 1;
                if(strlen(class_hint.res_class) > xwmain.width)
                    xwmain.width = strlen(class_hint.res_class);
                continue;
            }
        }

    }


    xwmain.width = ((xwmain.width)*(xwmain.font->max_advance_width))
                 + ((border_width*2));
    xwmain.height = ((xwmain.height)*(xwmain.font->ascent+xwmain.font->descent))
                  + ((xwmain.height)*(border_width*2));
    changes.width = xwmain.width;
    changes.height = xwmain.height;
    XConfigureWindow(xwmain.display,xwmain.winid,CWWidth|CWHeight,&changes);

    XMapWindow(xwmain.display,xwmain.winid);
    XSync(xwmain.display,false);

    while(1) {}
               
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
