#include <stdio.h>
#include <X11/Xlib.h>

#define OSD_VOLUME_IMPLEMENTATION
#include "osd_volume.h"

int main(int argc, char **argv)
{
    increase_volume();

    return(0);
}
