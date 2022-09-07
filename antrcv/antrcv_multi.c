#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "../../fs/include/params.h"     /* FS parameters            */
#include "../../fs/include/fs_types.h"   /* FS header files        */
#include "../../fs/include/fscom.h"      /* FS shared mem. structure */
#include "../../fs/include/shm_addr.h"   /* FS shared mem. pointer*/
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"   /* Station shared mem. pointer*/
struct fscom *fs;
struct stcom *st;

#define HELLO_PORT 1602 
#define HELLO_GROUP "224.168.2.132"
#define MSGBUFSIZE 16384

// typical packets: {"indvel": 0.0, "delphin-34": 7.849999904632568, "delphin-35": 7.899999618530273, ... }

void setup_ids();
void setup_st();
void logit();
void exit();
main(int argc, char *argv[])
{
//============================================
     struct sockaddr_in addr;
     int fd, nbytes,addrlen,i,j;
     char read_time[20];
     uint16_t channel,interface,interval;
     struct ip_mreq mreq;
     unsigned char msgbuf[MSGBUFSIZE];
     u_int yes=1;            
     int imesg,pkey,pdata,ikey,idata;
     char key[30],data[60];
     double vv;  //gets data numbers
     int iv,gstatus;     //integer data
     static int elaps,ioelaps,last_sentoff,last_ionsor,ln,lreached,stabletime;
     elaps=0; ioelaps=0; ln=0;

     setup_ids(); //FS and ST common
     setup_st();

     /* create what looks like an ordinary UDP socket */
     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
      perror("socket");
      exit(1);
     }


    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
       perror("Reusing ADDR failed");
       exit(1);
     }

     /* set up destination address */
     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
     addr.sin_port=htons(HELLO_PORT);
     
     /* bind to receive address */
     if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
      perror("bind");
      exit(1);
     }
     /* use setsockopt() to request that the kernel join a multicast group */
     mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);
     if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
      perror("setsockopt");
      exit(1);
     }
     /* enter endless read loop */
     while (1) {
      addrlen=sizeof(addr);
      if ((nbytes=recvfrom(fd,&msgbuf,sizeof(msgbuf),0,(struct sockaddr *) &addr,&addrlen)) < 0) {
           perror("recvfrom");
           exit(1);
      }
          if(msgbuf[0] == '{'){
//           printf("NBYTE=%d  \n",nbytes);
           imesg=0;pkey=0; pdata=0;idata=0;ikey=0;
           for(j=0;j <nbytes;j++){
             if(msgbuf[j] != ' '){           //skip spaces
//                printf("%c",msgbuf[j]);
                if((msgbuf[j] == ',') || (msgbuf[j] == '}')){        //new message or end of data?
                   key[pkey]='\0'; //terminate previous strings
                   data[pdata]='\0'; 
//                   printf("KEY,DATA=%s:%s\n",key,data);
//                   if(strcmp(key,"mjuld") == 0){printf("%d,time          : %s\n",imesg,data);}
//                   if(strcmp(key,"fe-ulo-frqs-1") == 0){sscanf(data,"%lf",&vv);printf("%d,fsynth1a: %f\n",imesg,vv);}
                   if(strcmp(key,"fe-ulo-frqs-0") == 0){sscanf(data,"%lf",&vv);stm_addr->fsynth1=vv*1000.0;} //need MHz
                   if(strcmp(key,"fe-ulo-frqs-2") == 0){sscanf(data,"%lf",&vv);stm_addr->fsynth2=vv*1000.0;}
//                   if(strcmp(key,"fe-rxfrq") == 0) {sscanf(data,"%lf",&vv);stm_addr->ant_rfcentre=vv*1000;} //int MHz
                   if(strcmp(key,"skyfreq") == 0) {sscanf(data,"%lf",&vv);stm_addr->ant_rfcentre=vv;} //int MHz
                   if(strcmp(key,"vpressure") == 0){ 
                       sscanf(data,"%lf",&vv);
                       if ((vv < 900.0) || (vv >999.9 ))vv=1000.0; /*eff problem*/
                       shm_addr->preswx=vv;
                   }
                   if(strcmp(key,"vtemperature") == 0){ sscanf(data,"%lf",&vv);shm_addr->tempwx=vv;} //use vv otherwise "nan" ?!
                   if(strcmp(key,"vhumidity") == 0){sscanf(data,"%lf",&vv); shm_addr->humiwx=vv;}
                   if(strcmp(key,"vwindvel") == 0){sscanf(data,"%lf",&vv); shm_addr->speedwx=vv;}
                   if(strcmp(key,"vwinddir") == 0){sscanf(data,"%lf",&vv); shm_addr->directionwx=vv;}
                   if(strcmp(key,"istmess") == 0){sscanf(data,"%d",&gstatus);} 
//gstatus=1 seems to be signal for onsource??
//                      if (gstatus==1) {
//                         shm_addr->ionsor=1 ; /*General status is measure -> on source*/
//                         stm_addr->antenna_sentoff=0; 
//                      }
//                      else shm_addr->ionsor=0;  /*Any other status -> off source*/
//                   }
        	   if((last_sentoff == 0) && (stm_addr->antenna_sentoff == 1)){
                	  elaps=0;   /*new source command, start counting secs */
                	  last_sentoff=1;
        	   }
        	   if(elaps == 10)stm_addr->antenna_sentoff=0; /*at latest clear new src request*/
        	   if ((elaps>4) && (gstatus==1)) {
                	 shm_addr->ionsor=1 ; /*General status is measure -> on source*/
                	 last_sentoff=0;
                	 stm_addr->antenna_sentoff=0; }
        	   else shm_addr->ionsor=0;  /*Any other status -> off source*/
        	   last_sentoff=stm_addr->antenna_sentoff;  
                   if(msgbuf[j] == '}')break;      //end of message
                   pkey=0;pdata=0;           //initl pointer to next pos in word
                   imesg++;
//                   printf (" MESSAGE NUMBER=%d\n",imesg);
                } else {
                   if(msgbuf[j] == '"'){ikey=1; idata=0;}  //says: now getting key
                   if(msgbuf[j] == ':'){ikey=0; idata=1;}  //says: now getting data
                   if((msgbuf[j] !='"') && (msgbuf[j] !=':')){
                     if(ikey == 1){
                        key[pkey]=msgbuf[j];
                        pkey++;
                     }
                     if(idata == 1){
                        data[pdata]=msgbuf[j];
                        pdata++;
                     }
                   }
                }
             }
           }
//           printf("elaps: %d last: %d antenna: %d status: %d onsource: %d \n",elaps,last_sentoff,stm_addr->antenna_sentoff,gstatus,shm_addr->ionsor);
       	   elaps++;
// 	   sleep(1);
          }
     }
}
