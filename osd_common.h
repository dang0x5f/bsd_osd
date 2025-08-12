#ifndef OSD_COMMON_H
#define OSD_COMMON_H

#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct node{
    void *node_data;
    struct node *next;
    struct node *prev;
}ListNode;

typedef struct list{
    void *head;
    void *tail;
    void *selected;
    size_t length;
}LinkList;

void *
xmalloc(size_t len);

XftFont *
font_setup(Display *display, 
           int screen_num, 
           const char *font_name);

void 
append_to_list(LinkList *list, 
               void *data);

#endif // OSD_COMMON_H

#ifdef OSD_COMMON_IMPLEMENTATION

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
    newnode = malloc(sizeof(ListNode));
    newnode->node_data = data;
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

#endif // OSD_COMMON_IMPLEMENTATION
