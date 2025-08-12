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

// red : #ff4040

void osd_proglist(void)
{
    XWindowChanges changes;

    XWindow_main xwmain = {0};
    LinkList list = {0};

    XContext context = XUniqueContext();

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

    /* query all toplevel windows and create list + nodes */
    uint32_t nchildren;
    Window root_ret, parent_ret, *children_ret;
    XQueryTree(xwmain.display,
               xwmain.root,
               &root_ret,
               &parent_ret,
               &children_ret,
               &nchildren);

    int ypos = 0, lineno = 0;
    int button_width = 0, button_height = 0;
    Window app_win;
    XWindow_app *app_btn = NULL;
    /* XTextProperty prop; */
    Atom actual_type;
    int32_t actual_format;
    uint64_t nitems, bytes_after;
    unsigned char *prop_ret = NULL; 
    XClassHint class_hint;
    Atom wm_state_atom = XInternAtom(xwmain.display, "WM_STATE",false);
    Atom wm_workspace_atom = XInternAtom(xwmain.display, "_NET_WM_DESKTOP",false);
    /* Atom wm_workspace_atom = XInternAtom(xwmain.display, "_NET_CURRENT_DESKTOP",false); */ // current visible workspace
Status status;
    for(size_t i=0; i<nchildren; ++i){

        XWindowAttributes attr;
        XGetWindowAttributes(xwmain.display,children_ret[i],&attr);
        if(!attr.override_redirect && attr.map_state == IsViewable){


            status = XGetWindowProperty(xwmain.display,
                                        children_ret[i],
                                        wm_workspace_atom,
                                        0, sizeof(uint32_t),
                                        false,
                                        AnyPropertyType,
                                        &actual_type, &actual_format,
                                        &nitems, &bytes_after,
                                        &prop_ret);
            if(prop_ret){
                printf("%d\n", (uint32_t)*prop_ret);
            }


            XGetClassHint(xwmain.display,children_ret[i],&class_hint);
            /* printf("res_name: %s\n",class_hint.res_name); */
            printf("res_class: %s\n",class_hint.res_class);
            /* XGetWMName(xwmain.display,children_ret[i],&prop); */
            /* printf("(%d) %s\n", children_ret[i],prop.value); */
            /* list.length += 1; */
            xwmain.height += 1;
            if(strlen(class_hint.res_class) > xwmain.width)
                xwmain.width = strlen(class_hint.res_class);

            ypos = (xwmain.font->ascent+xwmain.font->descent+
                   (border_width*2)) *
                   lineno;
            button_width = (strlen(class_hint.res_class)+2)*xwmain.font->max_advance_width;
            button_height = xwmain.font->ascent+xwmain.font->descent;

            app_btn = malloc(sizeof(XWindow_app));

            app_btn->name = malloc(sizeof(char)*strlen(class_hint.res_class));
            strncpy(app_btn->name,class_hint.res_class,strlen(class_hint.res_class));
            app_btn->namelen = strlen(class_hint.res_class);

            app_btn->button = create_button(xwmain.display,
                                           &xwmain.winid,
                                           &app_win,
                                           xwmain.depth,
                                           xwmain.visual,
                                           context,
                                           0,              // margin
                                           ypos,           // y
                                           button_width,     // width
                                           button_height,    // height
                                           &xwmain.colormap,
                                           2,
                                           0x333333,
                                           0xbbbbbb,
                                           "#000000",      // fg_color
                                           class_hint.res_class,
                                           strlen(class_hint.res_class),
                                           NULL,
                                           xwmain.font);

            ++lineno;
            append_to_list(&list,(void*)app_btn);

            continue;
        }

        status = XGetWindowProperty(xwmain.display,
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

                status = XGetWindowProperty(xwmain.display,
                                            children_ret[i],
                                            wm_workspace_atom,
                                            0, sizeof(uint32_t),
                                            false,
                                            AnyPropertyType,
                                            &actual_type, &actual_format,
                                            &nitems, &bytes_after,
                                            &prop_ret);
                if(prop_ret){
                    printf("%d\n", (uint32_t)*prop_ret);
                }


                XGetClassHint(xwmain.display,children_ret[i],&class_hint);
                /* printf("res_name: %s\n",class_hint.res_name); */
                printf("res_class: %s\n",class_hint.res_class);
                /* XGetWMName(xwmain.display,children_ret[i],&prop); */
                /* printf("(%d) %s\n", children_ret[i],prop.value); */
                /* list.length += 1; */
                xwmain.height += 1;
                if(strlen(class_hint.res_class) > xwmain.width)
                    xwmain.width = strlen(class_hint.res_class);

                ypos = (xwmain.font->ascent+xwmain.font->descent+
                       (border_width*2)) *
                       lineno;
                button_width = (strlen(class_hint.res_class)+2)*xwmain.font->max_advance_width;
                button_height = xwmain.font->ascent+xwmain.font->descent;

                app_btn = malloc(sizeof(XWindow_app));

                app_btn->name = malloc(sizeof(char)*strlen(class_hint.res_class));
                strncpy(app_btn->name,class_hint.res_class,strlen(class_hint.res_class));
                app_btn->namelen = strlen(class_hint.res_class);

                app_btn->button = create_button(xwmain.display,
                                               &xwmain.winid,
                                               &app_win,
                                               xwmain.depth,
                                               xwmain.visual,
                                               context,
                                               0,              // margin
                                               ypos,           // y
                                               button_width,     // width
                                               button_height,    // height
                                               &xwmain.colormap,
                                               2,
                                               0x333333,
                                               0xbbbbbb,
                                               "#000000",      // fg_color
                                               class_hint.res_class,
                                               strlen(class_hint.res_class),
                                               NULL,
                                               xwmain.font);

                ++lineno;

                append_to_list(&list,(void*)app_btn);

                continue;
            }
        }

    }
    /* end */

    size_t max_name_len = xwmain.width;
    size_t preferred_button_width = (max_name_len+2)*
                                    xwmain.font->max_advance_width;
    /* printf("%lu\n", preferred_button_width); */ 
    /* printf("%lu\n", list.length); */

    /* re-configure main width and height */
    xwmain.width = ((xwmain.width+2)*(xwmain.font->max_advance_width))
                 + ((border_width*2));
    xwmain.height = ((xwmain.height)*(xwmain.font->ascent+xwmain.font->descent))
                  + ((xwmain.height)*(border_width*2));
    changes.width = xwmain.width;
    changes.height = xwmain.height;
    XConfigureWindow(xwmain.display,xwmain.winid,CWWidth|CWHeight,&changes);
    /* end */

    /* re-configure button widths*/ 
    ListNode *iter = list.head;
    while(iter){
        printf("Button {\n  .width=%d\n  .height=%d\n}\n", 
                ((XWindow_app*)iter->node_data)->button->width,
                ((XWindow_app*)iter->node_data)->button->height);
        
        if(((XWindow_app*)iter->node_data)->button->width < preferred_button_width){

            ((XWindow_app*)iter->node_data)->button->width = preferred_button_width;

            XWindowChanges width_change;
            width_change.width = ((XWindow_app*)iter->node_data)->button->width;
            XConfigureWindow(xwmain.display,
                            ((XWindow_app*)iter->node_data)->button->win,
                            CWWidth,
                            &width_change); 
        }

        iter = iter->next;
    }
    /* end */

    
    /* init selected */
    // if head exists
    ListNode *node = list.head;
    select_button(((XWindow_app*)node->node_data)->button,
                    xwmain.display,
                    ((XWindow_app*)node->node_data)->button->win);
    list.selected = list.head;
    /* end */


    XMapWindow(xwmain.display,xwmain.winid);
    XSync(xwmain.display,false);


    XEvent ev;
    bool running=true;

    XGrabKeyboard(xwmain.display,
                  xwmain.winid,
                  true,
                  GrabModeAsync,
                  GrabModeAsync,
                  CurrentTime);
    while(running) {
        Button *button = NULL;
        XNextEvent(xwmain.display,&ev);
        XFindContext(ev.xany.display,ev.xany.window,context,(XPointer*)&button);
        switch(ev.type){
            case Expose:
                if(button)
                    expose_button(button,&ev);
                break;
            case KeyPress:
                printf("%d\n",ev.xkey.keycode);
                if(ev.xkey.keycode==9||ev.xkey.keycode==53)
                    running=false;
                break;
        }
    }
    XUngrabKeyboard(xwmain.display,
                    CurrentTime);
    
}

#endif 
