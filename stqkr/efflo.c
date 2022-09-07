#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT 256

/* replacement 'LO' command so that the frequency is sent out to 'antcn',
   this will ask the VAX via eff_rxvt to set the appropriate LO frequency */
/* New version Apr 99 to take care of LO=loa,86088.00,usb,rcp,pcal,offset,
 where parameters from rcp on only for VeEX */
void efflo(command,ip,isub,iresult)
struct cmd_ds *command;
long ip[5];
int isub,iresult;
{
   static float floa,flob;
   int i ,ierr;
   char wavel[20],suffix[2];
   char output[MAX_OUT];
   if(command->equal == '=') {
     if(command->argv[0] !=NULL){  sscanf(command->argv[1],"%f",&floa);}
                   else { fs->lo.lo[0]=0.0; fs->lo.lo[2]=0.0; floa=-1.0;}
     if(floa == -1.0) {
      ierr=-101;
      goto error;
     }
     ip[0]=100;  /* station -specific antcn mode */
     ip[1]=stm_addr->ieffcal;  /*cal word 0  or 1 */
     ip[2]=stm_addr->ieffpcal;  /*pcal word 0 or 1 */
/* look for lo values from schedule */
     if(strcmp(command->argv[0],"loa")==0 && floa != -1.0) fs->lo.lo[0]=floa;
     if(strcmp(command->argv[0],"lob")==0 && floa != -1.0) fs->lo.lo[1]=floa;
     if(strcmp(command->argv[0],"loc")==0 && floa != -1.0) fs->lo.lo[2]=floa;
     if(strcmp(command->argv[0],"lod")==0 && floa != -1.0) fs->lo.lo[3]=floa;
/*very inelegant section */
     if(strcmp(command->argv[0],"loa")==0 ) {
       fs->lo.sideband[0]=1; fs->lo.pol[0]=2; /*default*/
       if(strncmp(command->argv[2] ,"usb",3) == 0)fs->lo.sideband[0]=1;
       if(strncmp(command->argv[2] ,"lsb",3) == 0)fs->lo.sideband[0]=2;
       if(strncmp(command->argv[3] ,"rcp",3) == 0)fs->lo.pol[0]=1;
       if(strncmp(command->argv[3] ,"lcp",3) == 0)fs->lo.pol[0]=2;
       fs->lo.spacing[0]=1.0;   /*spacing MHz*/
       fs->lo.offset[0]=0.0;   
     }
     if(strcmp(command->argv[0],"lob")==0 ) {
       fs->lo.sideband[1]=1; fs->lo.pol[1]=2; /*default*/
       if(strncmp(command->argv[2] ,"usb",3) == 0)fs->lo.sideband[1]=1;
       if(strncmp(command->argv[2] ,"lsb",3) == 0)fs->lo.sideband[1]=2;
       if(strncmp(command->argv[3] ,"rcp",3) == 0)fs->lo.pol[1]=1;
       if(strncmp(command->argv[3] ,"lcp",3) == 0)fs->lo.pol[1]=2;
       fs->lo.spacing[1]=1.0;   /*spacing MHz*/
       fs->lo.offset[1]=0.0;
     }   
     if(strcmp(command->argv[0],"loc")==0 ) {
       fs->lo.sideband[1]=1; fs->lo.pol[1]=2; /*default*/
       if(strncmp(command->argv[2] ,"usb",3) == 0)fs->lo.sideband[2]=1;
       if(strncmp(command->argv[2] ,"lsb",3) == 0)fs->lo.sideband[2]=2;
       if(strncmp(command->argv[3] ,"rcp",3) == 0)fs->lo.pol[2]=1;
       if(strncmp(command->argv[3] ,"lcp",3) == 0)fs->lo.pol[2]=2;
       fs->lo.spacing[2]=1.0;   /*spacing MHz*/
       fs->lo.offset[2]=0.0;   
     }
     if(strcmp(command->argv[0],"lod")==0 ) {
       fs->lo.sideband[1]=1; fs->lo.pol[1]=2; /*default*/
       if(strncmp(command->argv[2] ,"usb",3) == 0)fs->lo.sideband[3]=1;
       if(strncmp(command->argv[2] ,"lsb",3) == 0)fs->lo.sideband[3]=2;
       if(strncmp(command->argv[3] ,"rcp",3) == 0)fs->lo.pol[3]=1;
       if(strncmp(command->argv[3] ,"lcp",3) == 0)fs->lo.pol[3]=2;
       fs->lo.spacing[3]=1.0;   /*spacing MHz*/
       fs->lo.offset[3]=0.0;   
     }
     skd_run("antcn",'w',ip);
     for (i=0;i<5;i++)ip[i]=0;
   }
   else  { /* report state */
     strcpy(output,command->name);
     strcat(output,"/");
     sprintf(wavel,"%5.2f , %5.2f",fs->lo.lo[0],fs->lo.lo[1]);    
     strcat (output,wavel);
     strcpy(wavel,",xxx");if (fs->lo.sideband[0] == 1)strcpy(wavel,",usb");
     if (fs->lo.sideband[0] == 2)strcpy(wavel,",lsb"); strcat (output,wavel);
     strcpy(wavel,",xxx");if (fs->lo.sideband[1] == 1)strcpy(wavel,",usb");
     if (fs->lo.sideband[1] == 2)strcpy(wavel,",lsb"); strcat (output,wavel);
     strcpy(wavel,",xxx");if (fs->lo.pol[0] == 1)strcpy(wavel,",rcp");
     if (fs->lo.pol[0] == 2)strcpy(wavel,",lcp"); strcat (output,wavel);
     strcpy(wavel,",xxx");if (fs->lo.pol[1] == 1)strcpy(wavel,",rcp");
     if (fs->lo.pol[1] == 2)strcpy(wavel,",lcp"); strcat (output,wavel);
     for (i=0;i<5;i++)ip[i]=0;
     cls_snd(&ip[0],output,strlen(output),0,0);
     ip[1]=1;
}
iresult=fs->lo.lo[0];  /* report lo state back to caller*/
return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"st",2);
    return;
}
