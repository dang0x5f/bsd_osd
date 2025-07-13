#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#define OSD_VOLUME_IMPLEMENTATION
#include "osd_volume.h"

int main(int argc, char **argv)
{
    display_volume_osd();

    return(0);
}
