#ifndef OSD_OUTMIXER_H
#define OSD_OUTMIXER_H

#include <sys/sysctl.h>
#include <mixer.h>

int osd_outmixer(void);         // Driver function
void *init_parameters(void);    // Setup window essentials
void *create_mixerlist(void);   // Retrieve list of mixer devices
void get_defaultunit(void);     
void set_defaultunit(void);
void *create_buttonlist(void);   

#endif

#ifdef OSD_OUTMIXER_IMPLEMENTATION

int osd_outmixer(void)
{
    struct mixer *m;
    struct mix_dev *md;
    char buffer[NAME_MAX];
    int n;
    
    if((n=mixer_get_nmixers())<0)
        errx(1,"No mixers present in system");
    for(int i=0; i<n; ++i){
        mixer_get_path(buffer, sizeof(buffer), i);

        if((m=mixer_open(buffer))==NULL) continue;

        printf("%s\n", m->name);
        printf("  - %s\n", m->ci.shortname);
        printf("  - %s\n", m->ci.longname);

        (void)mixer_close(m);
    }    

    return(EXIT_SUCCESS);
}

#endif
