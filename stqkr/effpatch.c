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


extern struct fscom *fs;
extern struct stcom *st;

#define MAX_OUT 256

/* replacement patch command, mk4 only, includes call to Eff patch device*/
void effpatch(command,ip,isub,iresult)
struct cmd_ds *command;
int ip[5];
int isub,iresult;
{
//   static int last_patch_set[16];
   int i ,ln,ivcn,iif,vnif,ierr,any_change;
    char vvv[4],vcn[3],vhl[2];
   char output[MAX_OUT];
   /*first for fun check what previous command has set patch array to:*/
//   for(i=0; i<16; i++)printf("%3i ",fs->ifp2vc[i]); 
//   printf("\n");
   if(command->equal == '=') {
     if(command->argv[0] ==NULL){
       for(i=0; i<16; i++){fs->ifp2vc[i]=0;} /*= without param=clear array*/
       for (i=0;i<5;i++)ip[i]=0;
       for (i=0; i<MAX_ARGS;i++)command->argv[i]=NULL; /*OOh thats brutal*/
       return;
     } else {
       strcpy(vvv,command->argv[0]);
       iif=1;   /*for patch=lo1 or lo2,lo3*/
       if(vvv[2] == '2')iif=2;
       if(vvv[2] == '3')iif=3;
//       printf("DEBptch0 %s %d \n",vvv,iif);
       for(i=1; i<16;i++){ /*1 to .. params are arg1 upwards*/
         if(command->argv[i] != NULL){
           ln=strlen(command->argv[i]);
//	   printf("DEBptch1 %d %d %s \n",i,ln,command->argv[i]);
	   if(ln > 1){
	     strncpy(vcn,command->argv[i],(ln-1));
	     vcn[ln-1]='\0';
	     sscanf(vcn,"%d",&ivcn);
	     strcpy(vvv,command->argv[i]);
	     if(vvv[(ln-1)] == 'h')strcpy(vhl,"h"); else strcpy(vhl,"l");
	     if(vvv[(ln-1)] == 'h')vnif=iif; else vnif=-iif;
	     fs->ifp2vc[ivcn-1]=vnif; /*ifp2vc[0] refers to VC1 */
//	     printf("DEBptch2 %d %d %c %d %d \n",i,ln,vvv[ln-1],ivcn,vnif);
//             printf("%s %d ",command->argv[i],ln);
//	     printf("> %s %s %d %d \n",vcn,vhl,ivcn,iif);
//	     printf("\n");
	   }
         }
       }
       /* check if patching has changed: if any setting asked for is non-zero, 
	  see if it has changed from last time. If no change, no need to call antcn 
	  with patch request. This avoids excessive relay changing with attendant
	  mechanical failure. The non-zero question also avoids changing when 
	  incomplete patch info available, eg after first line*/
       any_change=0;
       /*------debug print-------*/
//       printf("NEWP ");for(i=0; i<14;i++){printf("%d ",fs->ifp2vc[i]);} printf("\n");
//       printf("OLDP ");for(i=0; i<14;i++){printf("%d ",stm_addr->last_patch_set[i]);} printf("\n");
       /*------------------------*/
       for(i=0; i<14;i++){
         if(fs->ifp2vc[i] !=0){
           if(fs->ifp2vc[i] != stm_addr->last_patch_set[i]){
             any_change=1;
             stm_addr->last_patch_set[i]= fs->ifp2vc[i];
	   }
	 }
       }
       if(any_change != 0){
//         printf("WIND OF CHANGE ");
//         for(i=0; i<16;i++){printf("%d ",fs->ifp2vc[i]);}
//	 printf("\n");
         ip[0]=110;  /* station -specific antcn mode for patch panel*/
         ip[1]=0;
         ip[2]=0;
         skd_run("antcn",'w',ip);
       }
       for (i=0;i<5;i++)ip[i]=0;
     }
   }
   else  { /* report state */
     strcpy(output,command->name);
     strcat(output,"/");
//     sprintf(wavel,"%5.2f , %5.2f",fs->lo.lo[0],fs->lo.lo[1]);    
//     strcat (output,wavel);
//     strcpy(wavel,",xxx");if (fs->lo.sideband[0] == 1)strcpy(wavel,",usb");
//     if (fs->lo.sideband[0] == 2)strcpy(wavel,",lsb"); strcat (output,wavel);
//     strcpy(wavel,",xxx");if (fs->lo.sideband[1] == 1)strcpy(wavel,",usb");
//     if (fs->lo.sideband[1] == 2)strcpy(wavel,",lsb"); strcat (output,wavel);
//     strcpy(wavel,",xxx");if (fs->lo.pol[0] == 1)strcpy(wavel,",rcp");
//     if (fs->lo.pol[0] == 2)strcpy(wavel,",lcp"); strcat (output,wavel);
//     strcpy(wavel,",xxx");if (fs->lo.pol[1] == 1)strcpy(wavel,",rcp");
//     if (fs->lo.pol[1] == 2)strcpy(wavel,",lcp"); strcat (output,wavel);
     for (i=0;i<5;i++)ip[i]=0;
     cls_snd(&ip[0],output,strlen(output),0,0);
     ip[1]=1;
}
return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"st",2);
    return;
}
