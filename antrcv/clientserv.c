/* this receives from VAX and is used for WX and ONSOURCE */
/* this gets telescope/weather data from Effelsberg vax: 
   It looks something like this, with radec,azel,azelerr,press,temp,
   humidity and wind speed:
   "04_38_38.6___025_35_45___293_23_34___014_23_52__"
    012345678901234567890123456789012345678901234567  (0-47)
   "_000_00_00___000_00_00__970.4__15.6__92__2.2_"
    890123456789012345678901234567890123456789012     (48-92)
   Now a version which send info request to VAX and receives answer
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
#define EFFMK3PORT 51399
#define EFFVAXPORT 36010
#define SDHHOST "eff200"
#define ERROR  (-1)
#define ARGBUF 96
#define BUFDIM  256
#define MAXX 2
#include "../../fs/include/params.h"     /* FS parameters            */
#include "../../fs/include/fs_types.h"   /* FS header files        */
#include "../../fs/include/fscom.h"      /* FS shared mem. structure */
#include "../../fs/include/shm_addr.h"   /* FS shared mem. pointer*/
struct fscom *fs;
/* Subroutines called */
void setup_ids();
void logit();
long server_bind(long *mode,long *portnum,long *fsock,char *errortxt);
long socket_sendto(long *portnum,long *sock,char *buf,long *fbuflen
                  ,char *hostaddr);
long socket_rcvfrom(long *fsock,char *buf,long *fbuflen,char *clientaddr);
void bytezero(char *dest,int bytes);
void callerror(char *errortext,long severity);
long addr_file(char *filename,long *maxnr,long portcli[],char *hostaddr[]);
long check_address(long *maxnr,char *addrfeld[],char *iaddr);
/* antrcv main program starts here */
/* eff setup socket stuff */
/*---------------------------------------------------------------------*/
/* receive a message */
/*++******************************************************************/
main ()    
{
 long servport,cliport,mode,servsock,clisock,clilen;
 char clienttxt[BUFDIM],answertxt[BUFDIM];
 char *serverfile=ADRESSENFILE;
 BOOL senden=FALSE;  
 long icond,max=MAXX,cliports[MAXX],i,ianz;
 char *hosts[MAXX];
 char hostaddr[MAXX][80];

resock:                /*hhh restart here if system lost */

/* ----- Zuweisung der Adressenpointer  */
  for (i=0;i<MAXX;i++)
   { hosts[i]= &(hostaddr[i][0]); } 

/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.
*/
  setup_ids();
  fs = shm_addr;
  servport=36010;
  cliport=51399;
/*------  Socket fuer Server (send) aufsetzen und verbinden  */
  mode=1;
  icond=server_bind(&mode,&servport,&servsock,clienttxt);
  clilen=strlen(clienttxt)+1;

/*------  Socket fuer Client (receive) aufsetzen und verbinden  
          (nur wenn serverport und clientport verschieden sind) */
  mode=1;
  if (cliport != servport) 
   { icond=server_bind(&mode,&cliport,&clisock,clienttxt); 
   }
  else
   { clisock=servsock; }

/*------  Startup-Meldung senden  */
  icond=socket_sendto(&servport,&servsock,clienttxt,&clilen,"134.104.64.200");
  if ( icond < 0 ) { /*callerror(clienttxt,FATAL);*/      goto disaster; }
  if ( icond > 0 ) { callerror(clienttxt,NORMAL);     ianz= -1; }

    FOREVER
      int newfd,nread;
      int iddd,imm,iss,nw,n1,n2;
      char buf[ARGBUF];
      char ctemp[20],ctemp1[10];
      char hhhh[6],hhmm[6],hhss[6];
      char atpp[6],attp[6],athm[6],atwn[6];
      float vv,vv1;
    senden=FALSE;
    ianz= 1;
    strcpy(clienttxt,"SENDINFO");       
    clilen=strlen(clienttxt)+1;

/*------  Ueberpruefen, ob Anforderung vom Server gewuenscht */
      senden=TRUE;       ianz=2; 
    while (ianz>0)
    { 
/* -----  Text an server schreiben */
    icond=socket_sendto(&servport,&servsock,clienttxt,&clilen,"134.104.64.200");
    if ( icond < 0 ) { /*callerror(clienttxt,FATAL); */      goto disaster; }
    if ( icond > 0 ) { callerror(clienttxt,NORMAL);     ianz= -1; }

    if ( senden )
       { i=(int)BUFDIM;
         icond=socket_rcvfrom(&clisock,buf,&i,hosts[0]);
         if ( icond != 0 )
            { callerror(buf,NORMAL);   ianz= -1;  }
         else 
	    {  }
       }
        nread=strlen(buf);
    ianz -= 1;
    if (ianz <= 0) { senden=FALSE; }
        for (i=0; i< nread;i++) 
            { if(( buf[i] == '%' ) || ( buf[i] == ':') ) buf[i]=' '; }  /*vax*/
        if(buf[1] != '*') {
          n1=0; n2=strcut(buf,hhhh,' ',n1);  /* ra */
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1);
          n1=n2; n2=strcut(buf,hhhh,' ',n1); /*dec*/
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1); 
          n1=n2; n2=strcut(buf,hhhh,' ',n1); /*az*/
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1);
          n1=n2; n2=strcut(buf,hhhh,' ',n1); /*el*/
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1);
          n1=n2; n2=strcut(buf,hhhh,' ',n1); /*azer*/
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1);
          sscanf(hhhh,"%4d ",&iddd);sscanf(hhmm,"%2d ",&imm);sscanf(hhss,"%2d",&iss);
          if (  iddd < 0) { imm=-imm; iss=-iss;}
          iss=iss+(imm+iddd*60)*60; vv=iss;   /* in arcsec */
          n1=n2; n2=strcut(buf,hhhh,' ',n1); /*eler*/
          n1=n2; n2=strcut(buf,hhmm,' ',n1);
          n1=n2; n2=strcut(buf,hhss,' ',n1);
          sscanf(hhhh,"%4d ",&iddd);sscanf(hhmm,"%2d ",&imm);sscanf(hhss,"%2d",&iss);
          if (  iddd < 0) { imm=-imm; iss=-iss;}
          iss=iss+(imm+iddd*60)*60; vv1=iss;   /* in arcsec */
          if(vv < 0.0) vv=-vv;
          if(vv1 < 0.0) vv1=-vv1;
          /* if telescope within 30" of position say we are onsource,
             independent of frequency */
          if((vv < 30) && (vv1 <30)) fs->ionsor=1 ;
                                else fs->ionsor=0; 
          n1=n2; n2=strcut(buf,atpp,' ',n1); /* press*/
          /*if(strncmp(&atpp[2],"*",1))strcpy(atpp,"1000.0/0");*/ /*eff >999mb error*/
          n1=n2; n2=strcut(buf,attp,' ',n1); /* temp*/
          n1=n2; n2=strcut(buf,athm,' ',n1); /*hum*/
          n1=n2; n2=strcut(buf,atwn,' ',n1); /*wind*/
          sscanf(atpp,"%f",&vv); fs->preswx= vv ; 
          sscanf(attp,"%f",&vv); fs->tempwx= vv ; 
          sscanf(athm,"%f",&vv); fs->humiwx= vv; 
          }
       usleep(2000000);  /* 2 sec wait before next request */
      }
/* -----  Auf Ende ueberpruefen */
     if ( (icond=(int)strstr(clienttxt,"CLOSESOCKET")) != 0 )
        { close(servsock);   close(clisock);
          usleep(10000000);  /* 10 sec wait before next try */
          goto resock; }
      }
disaster: close(servsock); close (clisock); 
          usleep(10000000);  /* 10 sec wait before next try */
          goto resock;  /*hhhh*/
}
/* - - - - - - - - - - - - - - - - - - - -*/
int strcut(sin,sout,t,n)
char sin[],sout[],t; int n;
/* input is string sin, look for a string in this starting with
   (not ' '),terminated by t, copy to sout, null terminated and
    not including t . Start looking in sin at position n ,
    return finishing position*/
{
  int n1,n2,nn,i;
  n1=locaten(' ',sin,n); n2=locate(t,sin,n1);
  for (i=n1,nn=0; i<n2 ; i++,nn++) sout[nn]=sin[i]; sout[n2-n1]='\0';
  return (n2);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locate(t,s,n) /*find posn of char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]==t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locaten(t,s,n) /*find posn of not char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]!=t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/

long server_bind(long *fmode,long *portnum,long *fsock,char *errortxt)
/*                               Version 05.02.96 / jn */
{
 static struct sockaddr_in  server;

 int addrlen,sock,type,protocol,mode;
 long portnumber,iret;
 int x=1; /*hhhh*/

  portnumber=(long)*portnum;
  mode=(int)*fmode;

/*  Verbindungsmode festlegen   */

  type = SOCK_DGRAM; protocol = IPPROTO_UDP; 

/* ----- Oeffnen eines Internet-Sockets  */
  if ((sock = socket (AF_INET, type, protocol)) == INVALID_SOCKET)
     { iret=1; goto ende;  }                              /*=== ERROR ===*/
    
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&x, sizeof(x));

  *fsock=(long)sock;
  addrlen=sizeof(server);
  bytezero((char *)&server,addrlen);
  server.sin_family = AF_INET;
  server.sin_port = htons ((short)portnumber);
  server.sin_addr.s_addr = INADDR_ANY;
/* ----- Adresse des Servers ins System einbinden, damit alle Messages
         an den Server geleitet werden  */
  if (bind (sock, (struct sockaddr *) &server, addrlen) == SOCKET_ERROR)
     {  /*sprintf(errortxt," Cannot bind socket to system port %d " ,portnumber);
        callerror(errortxt,ADDSYSERR); */
        iret=1; goto ende;  }                             /*=== ERROR ===*/ 

  /*sprintf(errortxt ,"   \n\r Bind server on socket %d, port %ld !" ,sock,portnumber); */

  iret=0; 
ende:
  return(iret);
}

long socket_sendto(long *portnum,long *fsock,char *buf,long *fbuflen
                  ,char *hostaddr)
/*                               Version 05.02.96 / jn */
{
 struct sockaddr_in host; 

 int rval,flags,addrlen,buflen,sock;
 long portnumber,iret;

  sock=(int)*fsock;
  portnumber=(long)*portnum;
  buflen=(int)*fbuflen;
  flags=0;

/* ----- Verbinden zum Internet-server */
 
  addrlen=sizeof(host);
  bytezero((char *)&host,addrlen);
  host.sin_family = AF_INET;
  host.sin_port = htons ((short)portnumber);
  host.sin_addr.s_addr = inet_addr (hostaddr);
/* ------ Server sendet an Host mit Adresse hostaddr */

  rval = sendto(sock,buf,buflen,flags,(struct sockaddr *) &host,addrlen);

  if (rval == SOCKET_ERROR)
     { /*sprintf(buf,"Problems sending to host %s, port %d ", hostaddr,portnumber);
       callerror(buf,ADDSYSERR); */
       iret= -1; return(iret); }                            /*=== ERROR ===*/
  else
     { iret=0; return(iret); }


}

long socket_rcvfrom(long *fsock,char *buf,long *fbuflen
                   ,char *clientaddr)
/*                               Version 05.02.96 / jn */
{
 static struct sockaddr_in  client;
 int rval,flags,addrlen,buflen,sock;
 long iret;
  buflen=(int)*fbuflen;
  sock=(int)*fsock;
  flags=0;

/* ------ Server stellt Empfangsbereitschaft her und liest */
  addrlen=sizeof(client);
  bytezero((char *)&client,addrlen);
  fcntl(sock,F_SETFL,O_NONBLOCK);   /* make non-blocking */
  rval = recvfrom(sock,buf,buflen,flags,(struct sockaddr *)&client,&addrlen);
/* ------  Internet-Host-Adresse des rufenden Client uebertragen */

      strcpy(clientaddr,inet_ntoa(client.sin_addr) ); 
     if (rval  == SOCKET_ERROR)                                 /*=== ERROR ===*/
        { /*sprintf(buf," Error while receiving message from client %s " ,clientaddr);
          callerror(buf,ADDSYSERR);*/
          iret= -1; return(iret); }                           
     else 
        { iret=0; return(iret); }   
        iret=0; return(iret);   /* rval -1 just means no reply    */
}

long check_address(long *maxnr,char *addrfeld[],char *iaddr)
/*                               Version 05.02.96 / jn */
{
  int i,max,icond;
  long iret;
  max=(int)*maxnr;
  for (i=0;i<max;i++)
      { 
       if ( (icond=(int)strstr(addrfeld[i],iaddr)) != 0 ) 
          { iret=(long)i; return(iret); }                   /* gefunden */    
      }

  iret= -1;
  return(iret);
}

void callerror(char *errortext,long severity)
/*                               Version 23.01.96 / jn */
{
   switch (severity) 
     {
     case FATAL:   
       strcat(errortext," :: ");
       break;
     case INFORM:
       strcat(errortext," :: ");
       break;
     case ADDSYSERR:
       strcat(errortext," :: ");
       break;
     default:
       break;
     }
}
void bytezero(char *dest,int bytes)
{
  int i;

  for (i=0;i<bytes;i++)
     { dest[i]=(char)0; }
} 
