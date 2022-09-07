/* this receives from VAX and is used for WX and ONSOURCE */
/* this gets telescope/weather data from Effelsberg vax: 
   It looks something like this, with radec,azel,azelerr,press,temp,
   humidity and wind speed:
   "04_38_38.6___025_35_45___293_23_34___014_23_52__"
    012345678901234567890123456789012345678901234567  (0-47)
   "_000_00_00___000_00_00__970.4__15.6__92__2.2_"
    890123456789012345678901234567890123456789012     (48-92)
  This one receives broadcasts
   Modified so no print in it, can be started at bootup and runs
   in background.
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netinet/in.h>   /* sockaddr_in */
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include "sockdefs.h"
#define ERROR  (-1)
#define ARGBUF 96
#define BUFDIM  256
#define MAXX 2
/* the UDP message                   */
#define UDP_MSG_SIZE          2000
#define UDP_MSG_ADD_BYTES        9    /* 2 ints + the \0 */
#define UDP_UNIX_PORT        15004

 typedef struct UDPmessage
 {
  unsigned int   msglen;
  unsigned int   counter;
          char   message[UDP_MSG_SIZE];
  } UDP_MSG;           

#include "../../fs/include/params.h"     /* FS parameters            */
#include "../../fs/include/fs_types.h"   /* FS header files        */
#include "../../fs/include/fscom.h"      /* FS shared mem. structure */
#include "../../fs/include/shm_addr.h"   /* FS shared mem. pointer*/
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"   /* Station shared mem. pointer*/
struct fscom *fs;
struct stcom *st;
 struct sockaddr_in senderAddr;	
 struct sockaddr_in receiverAddr;
 UDP_MSG recmessage;
 int sockAddrSize;
 int sFd;		/* socket file descriptor */
 int status;
 char	inetAddr[20];
/* Subroutines called */
void setup_ids();
void setup_st();
void logit();
/* antrcv main program starts here */
/* eff setup socket stuff */
/*---------------------------------------------------------------------*/
/*++******************************************************************/
main ()    
{
 long servport,cliport,mode,servsock,clisock,clilen;
 char clienttxt[BUFDIM],answertxt[BUFDIM];
 char buf2[300];

 static int elaps,ioelaps,last_sentoff,last_ionsor,ln,lreached,stabletime;
 static int lax_ionsor,strict_ionsor;

 int len;
resock:                /*hhh restart here if system lost */
/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.
*/
  setup_ids();
//  fs = shm_addr;
  setup_st();
//  st = stm_addr;
 /* setup the local address */
 elaps=0; ioelaps=0; ln=0;
 sockAddrSize = sizeof(struct sockaddr_in);
 bzero ( (char *)&receiverAddr,sockAddrSize);
 receiverAddr.sin_family      = AF_INET;
 receiverAddr.sin_port        = htons((short)UDP_UNIX_PORT);
 receiverAddr.sin_addr.s_addr = htons((short)INADDR_ANY);

 /* create the UDP socket */ 
 if ( (sFd = socket(AF_INET,SOCK_DGRAM,0)) == ERROR)
  {
   return(ERROR);
   }
  
  /* bind the socket to local address */ 
  
  if ( (status = bind(sFd, (struct sockaddr *) &receiverAddr, sockAddrSize))
        == ERROR)
  {
   return(ERROR);
   }

    FOREVER
      int newfd,nread;
      int iddd,imm,iss,nw,n1,n2;
      int j,ezrsum,mlen;
      int gstatus;
      char buf[ARGBUF];
      char ctemp[20],ctemp1[10];
      char bf1[30],bf6[30];
      long scanmsec;
      float vv,vv1,vvtol;
      double vd;
      long vl;
      char *pp,*tmpch;
      int td1,td2,td3;
      float tf1;
      double synth1,synth2;
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
    status = recvfrom(sFd, (char *)&recmessage, sizeof(recmessage),0,
             (struct sockaddr *)&senderAddr,&sockAddrSize);
    if ( status == ERROR)
    {
      close (sFd);
      return(ERROR); 
     }
     if(strncmp(recmessage.message,"VMEFRONT_2",10) == 0){
       strncpy(bf1,&recmessage.message[15+25*4],25); bf1[25]='\0';
       sscanf(bf1,"%le",&synth1);
       strncpy(bf1,&recmessage.message[15+25*6],25); bf1[25]='\0';
       sscanf(bf1,"%le",&synth2);
       stm_addr->fsynth1=synth1/1.0e6;
       stm_addr->fsynth2=synth2/1.0e6;
       strncpy(bf1,&recmessage.message[15+25*26],25); bf1[25]='\0';
       sscanf(bf1,"%le",&vd);
       stm_addr->ant_rfcentre= vd;
//       printf(" %f \n",vd);
     }
     if(strncmp(recmessage.message,"VLBI_NEW",8) == 0){
       strncpy(bf1,&recmessage.message[15],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       stm_addr->ant_scanning= vv;
       strncpy(bf1,&recmessage.message[15+25*2],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       shm_addr->tempwx= vv ;
       strncpy(bf1,&recmessage.message[15+25*3],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       shm_addr->humiwx= vv ;
       strncpy(bf1,&recmessage.message[15+25*4],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       if ((vv < 900.0) || (vv >999.9 ))vv=1000.0; /*eff problem*/
       shm_addr->preswx= vv ;
       strncpy(bf1,&recmessage.message[15+25*5],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       shm_addr->speedwx= vv ;
       strncpy(bf1,&recmessage.message[15+25*6],25); bf1[25]='\0';
       sscanf(bf1,"%e",&vv);
       td1=vv;
       shm_addr->directionwx= td1 ;
       strncpy(bf6,&recmessage.message[15+25*9+10*3],10); bf6[10]='\0';
       sscanf(bf6,"%d",&gstatus);
//       printf(" %d \n",gstatus);
//       sprintf(buf2,"General Status is: %d",gstatus);
//       logit(buf2,0,NULL);
       if((last_sentoff == 0) && (stm_addr->antenna_sentoff == 1)){
          elaps=0;   /*new source command, start counting secs */
          last_sentoff=1;
       }
       elaps++;
//       printf("elaps: %d last: %d antenna: %d status: %d onsource: %d \n",elaps,last_sentoff,stm_addr->antenna_sentoff,gstatus,shm_addr->ionsor);
       if(elaps == 10)stm_addr->antenna_sentoff=0; /*at latest clear new src request*/
       if ((elaps>4) && (gstatus==4)) {
         shm_addr->ionsor=1 ; /*General status is measure -> on source*/
         last_sentoff=0;
         stm_addr->antenna_sentoff=0; }
       else shm_addr->ionsor=0;  /*Any other status -> off source*/
       last_sentoff=stm_addr->antenna_sentoff;  
// For testing 
//       else {
//         shm_addr->ionsor=1 ; /*General status is measure -> on source*/
//         stm_addr->antenna_sentoff=0; }
//       printf("Status %d Sent %d Onsour %d\n",gstatus,stm_addr->antenna_sentoff,shm_addr->ionsor);
//       sprintf(buf2,"Status %d Sent %d Onsour %d",gstatus,stm_addr->antenna_sentoff,shm_addr->ionsor);
//       logit(buf2,0,NULL);
     }
   }
}
/* - - - - - - - - - - - - - - - - - - - -*/
