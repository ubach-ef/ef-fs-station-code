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
# define MAXBUF 100
/*get gps value from hp counter and log it when asked for*/
void getgps(command,ip,isub,iresult)
struct cmd_ds *command;
int ip[5];
int isub,iresult;
{
   FILE *fp;
   char rxbuf[MAXBUF];
   char filename[MAXBUF];
   int i,ierr;
   char clkdiff[100];
   double m5bdiff;
   char output[MAX_OUT];
   sprintf(clkdiff," %+e" ,stm_addr->gpsdiff);    
   if((fp=fopen ("/home/oper/tmp.hp","r")) == (FILE *)NULL)
                                  printf(" cant open file \n");
   while (fgets(rxbuf,MAXBUF,fp) != NULL) {
   sscanf(&rxbuf[0],"%le",&m5bdiff);
//   printf("%+e\n",m5bdiff);
   sprintf(clkdiff," %+e" ,m5bdiff);
   if ((m5bdiff > 5e-4)) sprintf(clkdiff," %+e" ,m5bdiff-1); /*eff problem*/
   }
   strcpy(output,"fmout-gps");
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
