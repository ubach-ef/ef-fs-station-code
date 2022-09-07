/* antcn.c UDP for APEX*/
/* Input */
/* IP(1) = mode
       0 = initialize LU
       1 = pointing (from SOURCE command)
       4 = direct communications (from ANTENNA command)
       5 = on/off source status for pointing programs
100 - 32767 = for site specific use

   IP(2) = class number (mode 4 only)
   IP(3) = number of records in class (mode 4 only)
   IP(4) - not used
   IP(5) - not used
*/

/* Output */
/*  IP(1) = class with returned message
      (2) = number of records in class
      (3) = error number
            0 - ok
           -1 - illegal mode
           -2 - timeout
           -3 - wrong number of characters in response
           -4 - interface not set to remote
           -5 - error return from antenna
           -6 - error in pointing model initialization
            others as defined locally
      (4) = 2HAN for above errors, found in FSERR.CTL
          = 2HST for site defined errors, found in STERR.CTL
      (5) = not used
*/

/* Defined variables */
#define MINMODE 0  /* min,max modes for our operation */
#define MAXMODE 7

/* Include files */

#include <stdio.h>
#include <string.h>

#include "/usr2/fs/include/params.h" /* FS parameters            */
#include "/usr2/fs/include/fs_types.h" /* FS header files        */
#include "/usr2/fs/include/fscom.h"  /* FS shared mem. structure */
#include "/usr2/fs/include/shm_addr.h" /* FS shared mem. pointer */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// APEX port and machine, self for tests
#define APEXTRANSPORT 22122 
//#define APEXTRANSHOST "display2.mpifr-bonn.mpg.de"
#define APEXTRANSHOST "127.0.0.1"  

struct fscom *fs;

/* Subroutines called */
void setup_ids();
void putpname();
void skd_run(), cls_clr();
int nsem_test();
void logit();

/* antcn main program starts here */
main()
{
  int ierr, nrec, nrecr;
  int dum = 0;
  int r1, r2;
  int imode,i,nchar;
  long ip[5], class, clasr;
  char buf[80], buf2[300];
  char buf3[300];
  char sorname[20];
  static char outra[15], outdec[15]; /*check len of these! */

   int sockfd,n,iret,on,n1,n2,nepoch;
   struct sockaddr_in servaddr;
   fd_set in_fdset;
   struct itimerval value;

  char *host = APEXTRANSHOST;
  int port = APEXTRANSPORT;

/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.  */
  setup_ids();
  fs = shm_addr;
/* Put our program name where logit can find it. */
  putpname("antcn");
/*set up socket stuff*/
   sockfd=socket(AF_INET,SOCK_DGRAM,0);
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(APEXTRANSHOST);
   servaddr.sin_port = htons(port ? port : APEXTRANSPORT);
   on=1; setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on, sizeof(on));
   FD_ZERO(&in_fdset);
   FD_SET(sockfd,&in_fdset);
   value.it_value.tv_usec = 500000;//timeout 500 millisec on server reply
   value.it_value.tv_sec = 0;

/* Return to this point to wait until we are called again */
Continue:
  skd_wait("antcn",ip,(unsigned)0);

  imode = ip[0];
  class = ip[1];
  nrec = ip[2];

  nrecr = 0;
  clasr = 0;

  if (imode < MINMODE || imode > MAXMODE) {
    ierr = -1;
    goto End;
  }

/* Handle each mode in a separate section */
  switch (imode) {

    case 0:             /* initialize */
      ierr = 0;
      strcpy(buf,"Initializing antenna interface");
      logit(buf,0,NULL);
      fs->ionsor = 0;
      sprintf(buf3,"frontends(\'het230\')");
      break;

    case 1:             /* source= command */
      ierr = 0;
      strcpy(buf,"Commanding to a new source");
      rad2str(fs->ra50,"h0.4",outra); /*in our case this is J2000, not 1950*/
      rad2str(fs->dec50,"d0.4",outdec);
      n1=0; n2=strcut(fs->lsorna,sorname,' ',n1); /* look for space at end of source, max 10 char*/
      if(n2 > 9)n2=9; sorname[10]='\0';
      nepoch=fs->ep1950;
      if (nepoch < 1900)nepoch=1950;
      sprintf(buf2,"source\(\'%s\',\'%s\',\'%s\'\)\n",sorname,outra,outdec);
/*------------------------------------out via socket------*/
       sendto(sockfd,buf2,strlen(buf2),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
       usleep(10000); n=select(FD_SETSIZE,&in_fdset,NULL,NULL,&value.it_value);
       if(n !=0) { 
         n=recvfrom(sockfd,buf3,300,0,NULL,NULL);
         if(n <=0){strcpy(buf3,"REC_ERR"); n=8;} else {buf3[n]='\0';}
       } else {strcpy(buf3,"TIMEOUT");n=8; } 
/*--------------------------------------------------------*/
      logit(buf,0,NULL);
      fs->ionsor = 0;
      break;

    case 4:            /* direct antenna= command */
      if (class == 0)
        goto End;
      for (i=0; i<nrec; i++) {
        strcpy(buf2,"Received message for antenna: ");
        nchar = cls_rcv(class,buf,sizeof(buf),&r1,&r2,dum,dum);
	for(n1=0;n1<nchar;n1++){buf[n1]=tolower(buf[n1]);}
	buf[nchar]='\n'; //add LF for APEX
        buf[nchar+1] = '\0';  /* make into a string */
/*------------------------------------out via socket------*/
       sendto(sockfd,buf,strlen(buf),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
       usleep(10000); n=select(FD_SETSIZE,&in_fdset,NULL,NULL,&value.it_value);
       if(n !=0) { 
         n=recvfrom(sockfd,buf3,300,0,NULL,NULL);
         if(n <=0){strcpy(buf3,"REC_ERR"); n=8;} else {buf3[n]='\0';}
       } else {strcpy(buf3,"TIMEOUT");n=8; } 
/*--------------------------------------------------------*/
        strcat(buf2,buf3);
        logit(buf2,0,NULL);
        strcpy(buf,"ACK");
        cls_snd(&clasr,buf,3,dum,dum);
        nrecr += 1;
      }
      /* OR: cls_clr(class); */
      break;

    case 5:    /* onsource command with no error logging */
      ierr = 0;
//      strcpy(buf,"Checking onsource status, no error logging");
//      logit(buf,0,NULL);
      fs->ionsor = 1;
      break;

    case 6:            /* reserved */
      ierr = -1;
      goto End;

  }  /* end of switch */

End:
  ip[0] = clasr;
  ip[1] = nrecr;
  ip[2] = ierr;
  memcpy(ip+3,"AN",2);
  ip[4] = 0;
  goto Continue;

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

