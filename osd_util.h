#ifndef OSD_UTIL_H
#define OSD_UTIL_H

#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct node{
    void *data;
    struct node *next;
    struct node *prev;
} ListNode;
typedef struct ListNode ButtonNode;

typedef struct list{
    void *head;
    void *tail;
    void *selected;
    size_t length;
} LinkList;
typedef struct LinkList ButtonList;

typedef struct  {
    Display *display;
    int screen_num;
    XftDraw *draw;
    Colormap colormap;
    uint32_t depth;
    Visual *visual;
    uint32_t width;
    uint32_t height;
    uint32_t margin;
} WinResources;

typedef struct {
    Window id;
    /* WinData w;          TODO*/
    /* WinProcesses p;     TODO*/
    WinResources *r;
} OsdWindow;

void (*osd_func_init)(void);

void (*init)(void);
void (*win_init)(void);
void (*list_init)(void);
void (*indic_init)(void);

void (*draw)(void);
void (*draw_header)(void);
void (*draw_buttons)(void);

void (*run)(void);
void (*resize)(void);

void *
xmalloc(size_t len);

XftFont *
font_setup(Display *display, 
           int screen_num, 
           const char *font_name);

void 
append_to_list(LinkList *list, 
               void *data);
void
loop_list(LinkList *list);

#endif // OSD_UTIL_H

#ifdef OSD_UTIL_IMPLEMENTATION

void *
xmalloc(size_t len)
{
    void *p;
    
    if(!(p=malloc(len)))
        err(1,"malloc: %s\n",strerror(errno));

    return(p);
}

XftFont *
font_setup(Display* display, 
           int screen_num, 
           const char *font_name)
{
    XftFont *font = XftFontOpenName(display,
                                    screen_num,
                                    font_name);
    if(!font){
        perror("XftFontOpenName() error\n");
        exit(EXIT_FAILURE);
    }
    return(font);
}

void 
append_to_list(LinkList *list, 
               void *data)
{
    ListNode *newnode;
    newnode = xmalloc(sizeof(ListNode));
    newnode->data = data;
    newnode->next = NULL;

    if(list->head){
        ListNode *iter = list->head;
        while(iter->next) 
            iter = iter->next;
        
        newnode->prev = iter;
        iter->next = newnode;
    }else{
        newnode->prev = NULL;
        list->head = newnode;
    }
    
    list->tail = newnode;
    list->length += 1;
}

/* loop list to connect tail-to-head and head-to-tail */
void
loop_list(LinkList *list)
{
    ((ListNode*)list->head)->prev = (ListNode*)list->tail;
    ((ListNode*)list->tail)->next = (ListNode*)list->head;
}

WinResources *init_resources(void)
{
    WinResources *res = malloc(sizeof(WinResources));

    res->display    = XOpenDisplay(NULL);
    res->screen_num = DefaultScreen(res->display);
    res->colormap   = DefaultColormap(res->display,res->screen_num);
    res->depth      = DefaultDepth(res->display,res->screen_num);
    res->visual     = DefaultVisual(res->display,res->screen_num);

    return(res);
}

#define DPY_W 1
#define DPY_H 2

size_t getdim(Display *dpy, int dimension)
{
    static size_t width = 0;
    static size_t height = 0;

    switch(dimension){
        case DPY_W:
            if(width) return(width);

            width = DisplayWidth(dpy,DefaultScreen(dpy));
            if(width) return(width);
            
            err(1,"DisplayWidth() failed.\n");
            break;
        case DPY_H:
            if(height) return(height);

            height = DisplayHeight(dpy,DefaultScreen(dpy));
            if(height) return(height);
            
            err(1,"DisplayHeight() failed.\n");
            break;
        default:
            err(1,"getdim() failed.\n");
            break;
    }
}

#endif // OSD_UTIL_IMPLEMENTATION
