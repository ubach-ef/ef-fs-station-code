#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fscom.h"

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT 256
/*get gps value from truetime and log it when asked for*/
void getgps(command,ip,isub,iresult)
struct cmd_ds *command;
long ip[5];
int isub,iresult;
{
   int i,ierr;
   char clkdiff[100];
   char output[MAX_OUT];
   sprintf(clkdiff,"%3.4f" ,stm_addr->gpsdiff);    
   strcpy(output,command->name);
   strcat(output,"/");
   strcat (output,clkdiff);
   for (i=0;i<5;i++)ip[i]=0;
   cls_snd(&ip[0],output,strlen(output),0,0);
   ip[1]=1;
  return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"st",2);
    return;
}
