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

void effcal(command,ip,isub,iresult)
struct cmd_ds *command;
int ip[5];
int isub,iresult;
{
   static int ieffcal;
   int i ,ierr;
   char output[MAX_OUT];
   if(command->equal == '=') {
     if(command->argv[0] !=NULL){
       if(strcmp(command->argv[0],"on")==0)
       ieffcal=1;
       else
       if(strcmp(command->argv[0],"off")==0)
       ieffcal=0;
     }
     else {
      ierr=-104;
      goto error;
     }
     stm_addr->ieffcal=ieffcal;   
     ip[0]=100;  /* station -specific antcn mode */
     ip[1]=stm_addr->ieffcal ;  /*cal word 0  or 1 */
     ip[2]=stm_addr->ieffpcal;  /*pcal word 0 or 1 */
     ip[3]=stm_addr->ieffrx;  /*which rx wavelength */
     skd_run("antcn",'w',ip);
     for (i=0;i<5;i++)ip[i]=0;
   }
   else  { /* report state */
     strcpy(output,command->name);
     strcat(output,"/");
     if(ieffcal)
       strcat(output,"on");
     else
       strcat(output,"off");
     for (i=0;i<5;i++)ip[i]=0;
     cls_snd(&ip[0],output,strlen(output),0,0);
     ip[1]=1;
}
iresult=ieffcal;  /* report cal state back to caller*/
return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"st",2);
    return;
}
