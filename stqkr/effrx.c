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

void effrx(command,ip,isub,iresult)
struct cmd_ds *command;
long ip[5];
int isub,iresult;
{
   static int ieffrx;
   int i ,ierr;
   char wavel[6],suffix[2];
   char output[MAX_OUT];
   if(command->equal == '=') {
     if(command->argv[0] !=NULL) {
       if(strcmp(command->argv[0],"0cm")==0) ieffrx=0;  /* set by ibi */
       else
       if(strcmp(command->argv[0],"3mm")==0) ieffrx=85900;
       else
       if(strcmp(command->argv[0],"7mm")==0) ieffrx=42960;
       else
       if(strcmp(command->argv[0],"13mm")==0) ieffrx=22080;
       else
       if(strcmp(command->argv[0],"1cm")==0) ieffrx=22080;
       else
       if(strcmp(command->argv[0],"1.3cm")==0) ieffrx=22080;
       else
       if(strcmp(command->argv[0],"2cm")==0) ieffrx=15000;
       else
       if(strcmp(command->argv[0],"4cm")==0) ieffrx=8260;
       else
       if(strcmp(command->argv[0],"sx")==0) ieffrx=8110;
       else
       if(strcmp(command->argv[0],"geo")==0) ieffrx=8110;
       else
       if(strcmp(command->argv[0],"3.6cm")==0) ieffrx=8260;
       else
       if(strcmp(command->argv[0],"6cm")==0) ieffrx=4840;
       else
       if(strcmp(command->argv[0],"5cm")==0) ieffrx=6500;
       else
       if(strcmp(command->argv[0],"18cm")==0) ieffrx=1510;
       else
       if(strcmp(command->argv[0],"21cm")==0) ieffrx=1270;
       else
       if(strcmp(command->argv[0],"30cm")==0) ieffrx=650;
       else
       if(strcmp(command->argv[0],"50cm")==0) ieffrx=500;
     }
     else {
      ierr=-102;
      goto error;
     }
     stm_addr->ieffrx=ieffrx;   
     ip[0]=100;  /* station -specific antcn mode */
     ip[1]=stm_addr->ieffcal;  /*cal word 0  or 1 */
     ip[2]=stm_addr->ieffpcal;  /*pcal word 0 or 1 */
     ip[3]=stm_addr->ieffrx;  /*which RX (eigentlich LO freq)*/
     fs->lo.lo[0]=ieffrx;  /* this is the freq that's actually used,
                               overwritten by schedule lo */
     skd_run("antcn",'w',ip);
     for (i=0;i<5;i++)ip[i]=0;
   }
   else  { /* report state */
     strcpy(output,command->name);
     strcat(output,"/");
     sprintf(wavel,"%d",ieffrx);    
     strncpy(suffix,"cm",2);
     if(ieffrx == 3)strncpy(suffix,"mm",2);
     if(ieffrx == 7)strncpy(suffix,"mm",2);
     strncat (wavel,suffix,2);
     strcat (output,wavel);
     for (i=0;i<5;i++)ip[i]=0;
     cls_snd(&ip[0],output,strlen(output),0,0);
     ip[1]=1;
}
iresult=ieffrx;  /* report cal state back to caller*/
return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"st",2);
    return;
}
