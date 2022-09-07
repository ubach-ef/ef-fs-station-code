/*dummy telescope thing to test onoff*/
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
// long servport,cliport,mode,servsock,clisock,clilen;
// char clienttxt[BUFDIM],answertxt[BUFDIM];

 static int elaps,ioelaps,last_sentoff,last_ionsor,ln,lreached,stabletime;
 static int lax_ionsor,strict_ionsor;

 int len;
//resock:                /*hhh restart here if system lost */
/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.
*/
  setup_ids();
  setup_st();
 elaps=0; ioelaps=0; ln=0;
// sockAddrSize = sizeof(struct sockaddr_in);
// bzero ( (char *)&receiverAddr,sockAddrSize);
// receiverAddr.sin_family      = AF_INET;
// receiverAddr.sin_port        = htons((short)UDP_UNIX_PORT);
// receiverAddr.sin_addr.s_addr = htons((short)INADDR_ANY);

 /* create the UDP socket */ 
// if ( (sFd = socket(AF_INET,SOCK_DGRAM,0)) == ERROR)
//  {
//   return(ERROR);
//   }
  
  /* bind the socket to local address */ 
  
//  if ( (status = bind(sFd, (struct sockaddr *) &receiverAddr, sockAddrSize))
//        == ERROR)
//  {
//   return(ERROR);
//   }

    FOREVER
      int newfd,nread;
      int iddd,imm,iss,nw,n1,n2;
      int j,ezrsum,mlen;
      char buf[ARGBUF];
      char ctemp[20],ctemp1[10];
      char bf1[30],bf6[30];
      long scanmsec;
      float vv,vv1,vvtol;
      float aoff,eoff,toff,tof1,tof2;
      double vd;
      long vl;
      char *pp,*tmpch;
      int td1,td2,td3;
      float tf1;
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
      sleep(1); 
          if(stm_addr->antenna_sentoff == 1){
            if(last_sentoff == 0)elaps=0;
            stm_addr->ant_state = 1 ; /*stm_addr->ant_state 1 just told to move*/
            shm_addr->ionsor=0; /*immediate measure mark slewing*/
	    eoff=shm_addr->ELOFF*57.3; aoff=shm_addr->AZOFF*57.3;
	    if(eoff <0.0)eoff=-eoff; if(aoff <0.0)aoff=-aoff;
	    tof1=5+15.8*sqrt(eoff); tof2=8+15.8*sqrt(aoff);
	    toff=tof1; if(tof2 > toff)toff=tof2;
          }
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
         last_sentoff=stm_addr->antenna_sentoff;
         elaps++; ln++; 
         if(elaps >= 10)stm_addr->antenna_sentoff=0; /*at latest clear new src request*/
//---------------------------------------------------------------------------
//for dummy
          if(elaps < toff)vv=100;  else vv=1;
          if((vv < vvtol) && (vv1 < vvtol)) stm_addr->raw_ionsor=1 ; 
	                               else stm_addr->raw_ionsor=0; 
//---------------------------------------------------------------------------
//         if((stm_addr->antenna_sentoff == 1) && (last_sentoff == 0)){
//               stm_addr->ant_state = 1 ; /*stm_addr->ant_state 1 just told to move*/
//         }
         if((stm_addr->ant_state == 1) && (stm_addr->raw_ionsor == 0)){
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
	 /*could be we dont use any of that.. */
/*---------------------------------------------------------------------------------------*/
/*====if onof or fivpt are running report ionsor if "state" has reached 5  ====== */
          lax_ionsor=stm_addr->raw_ionsor;
	  if(stm_addr->ant_state >1)strict_ionsor=1; else strict_ionsor=0;
          if(stm_addr->antenna_sentoff == 1){strict_ionsor=0; lax_ionsor=0;}/*must really enforce this*/
          if ( (nsem_test("onoff") == 1) || (nsem_test("fivpt") == 1))shm_addr->ionsor=strict_ionsor;
	          else shm_addr->ionsor=lax_ionsor;
//  	          shm_addr->ionsor=stm_addr->raw_ionsor;

	  shm_addr->preswx= 950.0 ;
	  shm_addr->tempwx= 10.0 ;
	  shm_addr->humiwx= 50.0 ;
/* -----  Auf Ende ueberpruefen */
   }
}
/* - - - - - - - - - - - - - - - - - - - -*/
