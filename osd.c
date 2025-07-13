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

int main(int argc, char **argv)
{
    display_volume_osd(create_volume_lock(),argv[1][0]);

    return(0);
}
