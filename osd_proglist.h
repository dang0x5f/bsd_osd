#ifndef OSD_PROGLIST_H
#define OSD_PROGLIST_H

void osd_proglist(void);

#endif

#ifdef OSD_PROGLIST_IMPL

//  ...............
// [ * prog_name   ]
// [ * prog_nameee ]
// [+++++++++++++++]
// [ 1.prog_name   ]
// [>2.prog_nameee ]
// [ 3.prognam     ]
// [ 4.prog_name   ]
// [ 5.prog_nameee ]
// [ 6.prog_nameee ]
//  ```````````````

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

void xwmain_init(XWindow_main*);
void applist_init(XWindow_main*,XContext*,LinkList*);
Status get_atom_prop(XWindow_main*,Window,Atom,unsigned char**,size_t);
void reconf_xwmain_wh(XWindow_main*);
void resize_buttons(XWindow_main*,LinkList*,size_t);
void create_appnode(XWindow_main*,Window,Atom,XContext*,LinkList*,int*);
void debug_list_printout(LinkList*);

// red : #ff4040
static const int border_width = 2;

void osd_proglist(void)
{
    XWindow_main xwmain = {0};
    LinkList list = {0};

    XContext context = XUniqueContext();

    xwmain_init(&xwmain);

    /* query all toplevel windows and create list + nodes */
    applist_init(&xwmain,&context,&list);

    size_t max_name_len = xwmain.width;

    /* re-configure main width and height */
    reconf_xwmain_wh(&xwmain);

    /* re-configure button widths*/ 
    resize_buttons(&xwmain,&list,max_name_len);

/* debug_list_printout(&list); */
    
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
                
                bool redraw = false;
                KeySym keysym = XLookupKeysym(&ev.xkey,0);               

                switch(keysym){
                    case XK_Down:
                    case XK_j:
                        list.selected = ((ListNode*)list.selected)->next;
                        /* list->current_mixer = list->current_mixer->next; */
                        redraw=true;
                        break;
                    case XK_Up:  
                    case XK_k:
                        list.selected = ((ListNode*)list.selected)->prev;
                        /* list->current_mixer = list->current_mixer->prev; */
                        redraw=true;
                        break;
                    case XK_Return:
                        /* set_defaultunit(list->current_mixer->mixer_id,list); */
                        redraw=true;
                        break;
                    case XK_q:
                    case XK_Escape:
                        running=false;
                        break;
                }

                if(redraw){
                    ListNode *iter02 = list.head;
                    for(size_t i=0; i<list.length; ++i){
                        if(iter02 == list.selected)
                            select_button(((XWindow_app*)iter02->node_data)->button,
                                          xwmain.display,
                                          ((XWindow_app*)iter02->node_data)->button->win);
                        else
                            unselect_button(((XWindow_app*)iter02->node_data)->button,
                                            xwmain.display,
                                            ((XWindow_app*)iter02->node_data)->button->win);

                        iter02 = iter02->next;
                    }
                }

                printf("%d\n",ev.xkey.keycode);
                if(ev.xkey.keycode==9||ev.xkey.keycode==53)
                    running=false;
                break;
        }
    }
    XUngrabKeyboard(xwmain.display,
                    CurrentTime);
    
}

void 
applist_init(XWindow_main *xwmain, XContext *context, LinkList *list)
{
    uint32_t nchildren;
    Window root_ret, parent_ret, *children_ret;
    XQueryTree(xwmain->display,
               xwmain->root,
               &root_ret,
               &parent_ret,
               &children_ret,
               &nchildren);

    int lineno = 0;
    Status status;
    unsigned char *data = NULL; 
    Atom wm_state_atom = XInternAtom(xwmain->display, "WM_STATE",false);
    Atom wm_workspace_atom = XInternAtom(xwmain->display, "_NET_WM_DESKTOP",false);
    /* Atom wm_workspace_atom = XInternAtom(xwmain.display, "_NET_CURRENT_DESKTOP",false); */ // current visible workspace
    for(size_t i=0; i<nchildren; ++i){

        XWindowAttributes attr;
        XGetWindowAttributes(xwmain->display,children_ret[i],&attr);
        if(!attr.override_redirect && attr.map_state == IsViewable){

            create_appnode(xwmain, children_ret[i] , wm_workspace_atom , context, list, &lineno);
    
            continue;
        }

        status = get_atom_prop(xwmain, children_ret[i], wm_state_atom, &data, sizeof(int64_t));
        if(status == Success && data != NULL){
            int64_t *state_data = (int64_t*)data;
            int64_t state = state_data[0];

            if(state != WithdrawnState){

                create_appnode(xwmain, children_ret[i] , wm_workspace_atom , context, list, &lineno);

                continue;
            }
        }

    }
    ((ListNode*)list->head)->prev = (ListNode*)list->tail;
    ((ListNode*)list->tail)->next = (ListNode*)list->head;
}

void create_appnode(XWindow_main *xwmain, Window child, Atom atom, XContext *context, LinkList *list , int *lineno)
{
    int32_t ypos = 0;
    size_t button_width  = 0, 
           button_height = 0;
    unsigned char *data  = NULL;

    Window new_winid = 0;
    XWindow_app *new_button = NULL;
    XClassHint class_hint = {0};

    get_atom_prop(xwmain, child, atom, &data, sizeof(int32_t));
    if(data){
        printf("%d\n", (uint32_t)*data);
    }

    XGetClassHint(xwmain->display,child,&class_hint);
    /* printf("res_name: %s\n",class_hint.res_name); */
    printf("res_class: %s\n",class_hint.res_class);
    /* XGetWMName(xwmain.display,children_ret[i],&prop); */
    /* printf("(%d) %s\n", children_ret[i],prop.value); */
    /* list.length += 1; */
    xwmain->height += 1;
    if(strlen(class_hint.res_class) > xwmain->width)
        xwmain->width = strlen(class_hint.res_class);

    ypos = (xwmain->font->ascent+xwmain->font->descent+
           (border_width*2)) *
           (*lineno);
    button_width = (strlen(class_hint.res_class)+2)*
                   xwmain->font->max_advance_width;
    button_height = xwmain->font->ascent+
                    xwmain->font->descent;

    new_button = xmalloc(sizeof(XWindow_app));

    new_button->name = xmalloc(sizeof(char)*strlen(class_hint.res_class));
    strncpy(new_button->name,class_hint.res_class,strlen(class_hint.res_class));
    new_button->namelen = strlen(class_hint.res_class);

    new_button->button = create_button(xwmain->display,
                                   &xwmain->winid,
                                   &new_winid,
                                   xwmain->depth,
                                   xwmain->visual,
                                   *context,
                                   0,                // margin
                                   ypos,             // y
                                   button_width,     // width
                                   button_height,    // height
                                   &xwmain->colormap,
                                   2,
                                   0x333333,
                                   0xbbbbbb,
                                   "#000000",      // fg_color
                                   class_hint.res_class,
                                   strlen(class_hint.res_class),
                                   NULL,
                                   xwmain->font);

    ++(*lineno);

    append_to_list(list,(void*)new_button);
}

Status 
get_atom_prop(XWindow_main *xwmain, Window child, Atom atom, unsigned char **data, size_t nbytes)
{
    Status status;
    Atom actual_type = {0};
    int32_t actual_format = {0};
    uint64_t nitems = {0}, bytes_after = {0};
    status = XGetWindowProperty(xwmain->display,
                                child,
                                atom,
                                0, nbytes,
                                false,
                                AnyPropertyType,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after,
                                data);

    return(status);
}

void xwmain_init(XWindow_main *xwmain)
{
    xwmain->display = XOpenDisplay(NULL); 
    xwmain->screen = DefaultScreen(xwmain->display);
    xwmain->root = DefaultRootWindow(xwmain->display);
    xwmain->font = font_setup(xwmain->display, xwmain->screen,
                             "Deja Vu Sans Mono:pixelsize=16");
                             /* "IBM Plex Mono:pixelsize=16"); */
    xwmain->depth = DefaultDepth(xwmain->display, xwmain->screen);
    xwmain->visual = DefaultVisual(xwmain->display, xwmain->screen);
    xwmain->colormap = DefaultColormap(xwmain->display, xwmain->screen);
    xwmain->screen_width = DisplayWidth(xwmain->display,xwmain->screen);
    xwmain->screen_height = DisplayHeight(xwmain->display,xwmain->screen);

    int x=0, y=0;
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
    xwmain->winid = XCreateWindow(xwmain->display, 
                                  xwmain->root, 
                                  x, y, 
                                  xwmain->screen_width, 
                                  xwmain->screen_height, 
                                  border_width, 
                                  xwmain->depth, 
                                  InputOutput, 
                                  xwmain->visual, 
                                  valuemask, 
                                  &attributes);
}

void reconf_xwmain_wh(XWindow_main *xwmain)
{
    XWindowChanges changes;
    xwmain->width = ((xwmain->width+2)*(xwmain->font->max_advance_width))
                 + ((border_width*2));
    xwmain->height = ((xwmain->height)*(xwmain->font->ascent+xwmain->font->descent))
                  + ((xwmain->height)*(border_width*2));
    changes.width = xwmain->width;
    changes.height = xwmain->height;
    XConfigureWindow(xwmain->display,xwmain->winid,CWWidth|CWHeight,&changes);
}

void resize_buttons(XWindow_main *xwmain, LinkList *list, size_t max_name_len)
{
    size_t preferred_button_width = (max_name_len+2)*
                                    xwmain->font->max_advance_width;
    ListNode *iter = list->head;
    for(size_t i=0; i<list->length; ++i){
        printf("Button {\n  .width=%d\n  .height=%d\n}\n", 
                ((XWindow_app*)iter->node_data)->button->width,
                ((XWindow_app*)iter->node_data)->button->height);
        
        if(((XWindow_app*)iter->node_data)->button->width < preferred_button_width){

            ((XWindow_app*)iter->node_data)->button->width = preferred_button_width;

            XWindowChanges width_change;
            width_change.width = ((XWindow_app*)iter->node_data)->button->width;
            XConfigureWindow(xwmain->display,
                            ((XWindow_app*)iter->node_data)->button->win,
                            CWWidth,
                            &width_change); 
        }

        iter = iter->next;
    }
}

void debug_list_printout(LinkList *list)
{
    printf("--------------------------\n");
    ListNode *iter01 = list->head;
    for(size_t i=0; i<list->length; ++i){
        printf("%lu\n {\n  curr: %p\n  prev: %p\n  next: %p\n }\n\n",
               i, (void*)iter01, (void*)iter01->prev, (void*)iter01->next);

        iter01 = iter01->next;
    }
    printf("--------------------------\n");
}

#endif 
