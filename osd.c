#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#define OSD_VOLUME_IMPLEMENTATION
#include "osd_volume.h"

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
    while((opt=getopt(argc,argv,"v:h?")) != -1){
        switch(opt){
            case 'v':
                osd_volume(optarg[0]);
                break;
            case 'h':
            case '?':
            default:
                usage(argv[0]);
        }
    }

    return(EXIT_SUCCESS);
}
