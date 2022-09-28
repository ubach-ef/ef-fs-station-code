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
 int servport,cliport,mode,servsock,clisock,clilen;
 char clienttxt[BUFDIM],answertxt[BUFDIM];

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
      char buf[ARGBUF];
      char ctemp[20],ctemp1[10];
      char bf1[30],bf6[30];
      int scanmsec;
      float vv,vv1,vvtol;
      double vd;
      int vl;
      char *pp,*tmpch;
      int td1,td2,td3;
      float tf1;
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
    status = recvfrom(sFd, (char *)&recmessage, sizeof(recmessage),0,
             (struct sockaddr *)&senderAddr,&sockAddrSize);
    if ( status == ERROR)
    {
      close (sFd);
      return(ERROR); 
     }
     if(strncmp(recmessage.message,"VAXTIME",7) == 0){
       strncpy(bf6,&recmessage.message[12+10*5],10); bf6[10]='\0';
       sscanf(bf6,"%xl",&scanmsec);
       strncpy(bf1,&recmessage.message[12+10*9+25*9],25); bf1[25]='\0';
     }
     if(strncmp(recmessage.message,"VLBIDATA",8) == 0){
 /*VLBIDATA   14 09 33.6  052 26 13  337 11 20  017 24 44  158 59 46 -072 35 17 977.4  19.5  40%  1.0 */    
          strcat(recmessage.message,bf1);
          strcat(recmessage.message,bf6);
          mlen=strlen(recmessage.message);
          pp=(char *)strtok(recmessage.message," ");
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*ra */
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%f",&tf1); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*dec*/
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%f",&tf1); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*az*/
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td3); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*el*/
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td3); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*azer*/
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td3); 
	  if(td1 < 0){td2=-td2; td3=-td3; }
          td3=td3+(td2+td1*60)*60; vv=td3;   /* in arcsec */
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td1); /*eler*/
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td2); 
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%d",&td3); 
	  if(td1 < 0){td2=-td2; td3=-td3; }
          td3=td3+(td2+td1*60)*60; vv1=td3;   /* in arcsec */
          if(vv < 0.0) vv=-vv;
          if(vv1 < 0.0) vv1=-vv1;
          vvtol=10.0;  /*tolerance arcsec*/
          if((vv < vvtol) && (vv1 < vvtol)) stm_addr->raw_ionsor=1 ; 
	                               else stm_addr->raw_ionsor=0; 
          /* if telescope within 30 arcsec of position local onsource
	    indicator =1 independent of frequency */
/*------------------------------------ state thing to see how stable antenna position is */
         if((last_sentoff == 0) && (stm_addr->antenna_sentoff == 1)){
            elaps=0;   /*new source command, start counting secs */
         }
         /*ioelaps = how long has ionsor been "on" */
         if (stm_addr->raw_ionsor == 0)ioelaps=0; else ioelaps++;
         elaps++; ln++; 
         if(elaps == 10)stm_addr->antenna_sentoff=0; /*at latest clear new src request*/
//---------------------------------------------------------------------------
         if((stm_addr->antenna_sentoff == 1) && (last_sentoff == 0)){
               stm_addr->ant_state = 1 ; /*stm_addr->ant_state 1 just told to move*/
         }
//         if((stm_addr->ant_state == 1) && (stm_addr->raw_ionsor == 0)){
         if((stm_addr->antenna_sentoff == 1) && (stm_addr->raw_ionsor == 0)){
               stm_addr->ant_state = 2 ; /*stm_addr->ant_state 2 told to move and started move*/
               stm_addr->antenna_sentoff=0; /* clear new src request*/
         }
         if((ioelaps >9 ) && (stm_addr->ant_state <3) && (stm_addr->raw_ionsor == 1)){
               stm_addr->ant_state = 3 ; /*stm_addr->ant_state 3 by defaultreached source*/
               lreached=ln;
         }
         if((stm_addr->ant_state == 2) && (stm_addr->raw_ionsor == 1)){
               stm_addr->ant_state = 3 ; /*stm_addr->ant_state 3 reached source,may still wobble*/
               lreached=ln;
         }
         if(stm_addr->raw_ionsor == 0){
               stm_addr->ant_state = stm_addr->ant_state-1 ; /*wobbled off so back to stm_addr->ant_state 1*/
         }
         if((stm_addr->ant_state == 3) && (stm_addr->raw_ionsor == 1)){
               stabletime=ln-lreached;
               if(stabletime > 3){
                 stm_addr->ant_state = 4 ; /*stm_addr->ant_state 4 solidly on source*/
               }
         }
         last_sentoff=stm_addr->antenna_sentoff;  
	 /*could be we dont use any of that.. */
/*---------------------------------------------------------------------------------------*/
/*====if onof or fivpt are running report ionsor if "state" has reached 4  ====== */
          lax_ionsor=stm_addr->raw_ionsor;
	  if(stm_addr->ant_state >2)strict_ionsor=lax_ionsor; else strict_ionsor=0;
          if(stm_addr->antenna_sentoff == 1)strict_ionsor=0; /*just to make sure..*/
          if ( (nsem_test("onoff") == 1) || (nsem_test("fivpt") == 1))shm_addr->ionsor=strict_ionsor;
	          else shm_addr->ionsor=lax_ionsor;
//  	          shm_addr->ionsor=stm_addr->raw_ionsor;

          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%f",&vv); 
	  if ((vv < 900.0) || (vv >999.9 ))vv=1000.0; /*eff problem*/
	  shm_addr->preswx= vv ;
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%f",&vv); shm_addr->tempwx= vv ;
          tmpch=(char *)strtok(0," ");sscanf(tmpch,"%f",&vv); shm_addr->humiwx= vv ;
          tmpch=(char *)strtok(0," "); /*wind unused*/
	  if(mlen > 120){
            tmpch=(char *)strtok(0," "); /*frequency*/
	    sscanf(tmpch,"%le",&vd); 
	    vl=(vd+0.00001)*1000.0;
	    stm_addr->ant_rfcentre= vl;
            tmpch=(char *)strtok(0," "); /*scanmsc*/
	    sscanf(tmpch,"%xl",&vl); 
	    stm_addr->ant_scanning= vl;
//	    printf(" %d %ld:%s\n",stm_addr->ant_rfcentre,stm_addr->ant_scanning,stm_addr->ant_command);
          }
/* new params at end, Aug 2005:
02 21 56.0  061 52 42  321 05 04  041 11 44  280 56 09 -048 48 17 977.9  21.3  60%  3.1      0.1460000000000000E+02  FFFFFC18
*/
      }
/* -----  Auf Ende ueberpruefen */
   }
}
/* - - - - - - - - - - - - - - - - - - - -*/
