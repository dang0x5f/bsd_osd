#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <sys/sysctl.h>

#define OSD_COMMON_IMPLEMENTATION
#include "osd_common.h"

#define OSD_VOLUME_IMPLEMENTATION
#include "osd_volume.h"

#define OSD_OUTMIXER_IMPLEMENTATION
#include "osd_outmixer.h"

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
    while((opt=getopt(argc,argv,"m:v:h?")) != -1){
        switch(opt){
            case 'v':
                osd_volume(optarg[0]);
                break;
            case 'm':
                osd_outmixer();
                break;
            case 'h':
            case '?':
            default:
                usage(argv[0]);
        }
    }

    return(EXIT_SUCCESS);
}
