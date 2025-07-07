#ifndef OSD_VOLUME_H
#define OSD_VOLUME_H

/* VOLUME            20 */
/* ||||---------------- */

/* VOLUME            45 */
/* |||||||||----------- */

/* VOLUME            20 */
/* ████________________ */

/* VOLUME            80 */
/* ████████████████____ */

#include <err.h>
#include <mixer.h>

void increase_volume(void);

#endif // OSD_VOLUME_H

#ifdef OSD_VOLUME_IMPLEMENTATION

void increase_volume(void)
{

}

void increase_volume1(void)
{
    printf("Hello, World!\n");
    struct mixer *m;
    mix_volume_t vol;
    char *mix_name, *dev_name;

    mix_name = NULL;
    if((m=mixer_open(mix_name))==NULL)
        err(1,"mixer_open: %s", mix_name);

    dev_name = "vol";
    if((m->dev=mixer_get_dev_byname(m,dev_name))<0)
        err(1,"unknown device: %s", dev_name);

    printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 

    vol.left = m->dev->vol.left + 0.01;
    vol.right = m->dev->vol.right + 0.01;
    mixer_set_vol(m,vol);
    // if(mixer_set_vol(m,vol) < 0)
    //     warn("cannot change volume");
    
    printf("left: %0.2f right: %0.2f\n", m->dev->vol.left,m->dev->vol.right); 
}


#endif // OSD_VOLUME_IMPLEMENTATION
