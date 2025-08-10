#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <err.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>

#include <sys/sysctl.h>

#define OSD_COMMON_IMPLEMENTATION
#include "osd_common.h"

#define OSD_VOLUME_IMPLEMENTATION
#include "osd_volume.h"

#define OSD_OUTDEVICE_IMPLEMENTATION
#include "osd_outdevice.h"

#define OSD_PROGLIST_IMPL
#include "osd_proglist.h"

void usage(char *progname)
{
    printf("usage:\n  %s -v %s\n",
            progname, "[d/-/u/+]");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if(argc < 2) usage(argv[0]);

    int opt;
    while((opt=getopt(argc,argv,"lm:v:h?")) != -1){
        switch(opt){
            case 'v':
                osd_volume(optarg[0]);
                break;
            case 'm':
                if(optarg[0]=='o')
                    osd_outdevice();
                break;
            case 'l':
                osd_proglist();
                break;
            case 'h':
            case '?':
            default:
                usage(argv[0]);
        }
    }

    return(EXIT_SUCCESS);
}
