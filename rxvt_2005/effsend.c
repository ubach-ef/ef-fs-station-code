/* eff eff effe eff this receives SOURCE commands on a socket
   from FS9 and VLBA system, passes them on to whatever program
   is being run by rxvt, e.g. a telnet to OBSE on eff200..,
   and also looks after the single=board computer in mk3 IFD*/
   /* 2004 modifications marked with year */
   /*2005 for new machine */
#include <stdio.h>
# define MAXBUF 100
static int rxbits=0x00;
static int pcalmask=0x40; /* keep in here if pcal is sf,pf or if */
void ifk();
void sfk();
void pfk();
void s7cal ();
void rxno();
void wrbits();
int send_effberg(unsigned char *buf,int count)
{
  FILE *fp;
  char rxbuf[MAXBUF];
  char filename[MAXBUF];
  int nread,n1,n2,n,i,nn,n3,j;
  int irxno,irxlam;
  float flo,rxfq; /*replaces ilo*/
  static int devcua=-1;
  char expcode[20],srcname[20],ra[20],dec[20],band[20] ;
  char noise[20],pcal[20],effxtra[20];
  char vori[6];  /* gets VLBI for Mk3, VLBA for VLBA */
  char vork[6];  /* gets VVLBI for Mk3, VVLBA for VLBA */
  int inoise,ipcal;   /* store here whether on or off wanted*/
  float efflo,ftest;  /* translate band to this,  LO frequency (was integer) */
  static char ora[20],odec[20],oband[20],otopo[10] ;
  char rah[4],ram[4],ras[8],decd[4],decm[4],decs[8], whichsys[20];
  char obuf[1024];
  char nearest[200]; /*2004*/
  float fdiff,fdf; /*2004*/
/* was ttyS16 for other machine, now extra interface */
  if (devcua == -1){devcua=open("/dev/ttyS4",02);}
  buf[count]='\0';
/* unless this starts with >>, it's antenna= command to put straight out*/
  if ((buf[0] !='>') || (buf[1] != '>') ) { 
        cprintf("%s\r\n",buf);
        return;
     }
  /* first check if this is from VME or FS9, FS9 has >>SM in field 1 
     for a source command, VLBA has >>SV*/
     n1=0; n2=strcut(buf,whichsys,' ',n1);
     if( strncmp(whichsys,">>SV",4) == 0){strcpy(vori,"VLBA"); /*was @VLBA */
                                          strcpy(vork,"VVLBA"); }
     if( strncmp(whichsys,">>SM",4) == 0){strcpy(vori,"VLBI"); 
                                          strcpy(vork,"VVLBI"); }
  /* section :separate input line into command words , */
     n1=n2; n2=strcut(buf,expcode,' ',n1);
     n1=n2; n2=strcut(buf,srcname,' ',n1);
     n1=n2; n2=strcut(buf,rah,'h',n1);
     n1=n2+1; n2=strcut(buf,ram,'m',n1);
     n1=n2+1; n2=strcut(buf,ras,'s',n1);
     n1=n2+1; n2=strcut(buf,decd,'d',n1);
     n1=n2+1; n2=strcut(buf,decm,'\'',n1);
     n1=n2+1; n2=strcut(buf,decs,'"',n1);
     n1=n2+1; n2=strcut(buf,band,' ',n1);
     n1=n2+1; n2=strcut(buf,noise,' ',n1);
     n1=n2+1; n2=strcut(buf,pcal,' ',n1);
     n1=n2+1; n2=locaten(' ',buf,n1);
     strncpy(effxtra,&buf[n2],7);
/*cprintf("EFFXTRA=:%s:\n",effxtra);*/
     strcpy(ra,rah); strcat(ra,ram); strcat(ra,ras);
     strcpy(dec,decd); strcat(dec,decm); strcat(dec,decs);
     if ((strncmp(ra,ora,10) != 0 ) || ( strncmp(dec,odec,10) != 0 )) {
       strncpy(ora,ra,10); strncpy(odec,dec,10);
       if(strncmp(srcname,"NONE",4) !=0)  {
         cprintf(" \r\n");  /* in case observer was typing something, abschliessen */
         cprintf("OBSINP \r\n");
         cprintf("SNAM=%s\r\n",srcname);
         cprintf("SLAM=%s %s %s S\r\n",rah,ram,ras);
         cprintf("SBET=%s %s %s \r\n",decd,decm,decs);
         if(strncmp(effxtra,"2000",4)== 0) cprintf("SBAS=-1\r\n");
         if(strncmp(effxtra,"1950",4)== 0) cprintf("SBAS=1\r\n");
/* FS9 message now has FSX (neutral),FSN (north), FSS (south ) at end */
         if(strncmp(&effxtra[6],otopo,1) != 0) {
           if(strncmp(&effxtra[6],"X",1)== 0) cprintf("TOPO=AUTO\r\n");
           if(strncmp(&effxtra[6],"N",1)== 0) cprintf("TOPO=NORTH\r\n");
           if(strncmp(&effxtra[6],"S",1)== 0) cprintf("TOPO=SOUTH\r\n");
           strncpy(otopo,&effxtra[6],1);
         }
         cprintf("QUIT\r\n");
         cprintf("%s\r\n",vork);
       }
     }
     for (i=0; i< strlen(band); i++)band[i]=toupper(band[i]); 
     sscanf(band,"%f",&efflo);      /* lo to float (was integer) */
     /*May 05 added check to makesure impossible LOs not used, eg FS start*/
     if( (strcmp(band,"NONE") != 0) && (efflo > 200.0)){
       if ((strcmp(band,oband) != 0) ) {
         if((fp=fopen ("/usr2/control/rxfile","r")) == (FILE *)NULL)
                                         printf(" cant open file \n");
//         sscanf(band,"%f",&efflo);      /* lo to float (was integer) */
 	 fdiff=1.0e6; /*2004*/
         while (fgets(rxbuf,MAXBUF,fp) != NULL) {
           if(rxbuf[0] == whichsys[3] ){       /* first check if VLBA or MK3 setup*/
             sscanf(&rxbuf[1],"%f",&flo);      /* lo from list */
	     /* 2004------*/
	     fdf=(efflo-flo); 
	     /* incase not found in our fixed list, find nearest match*/
	     if(fabs(fdf) < fabs(fdiff)){
                strcpy(nearest,rxbuf);
		fdiff=fdf;
	     }
           }                                               
	  /* 2004-end---*/
         }                                               
         fclose(fp);
         /* ideally fdiff=0 and this is exact match.
	   if not found in list, have to estimate what's needed */
         sscanf(&nearest[12],"%d %d %f ",&irxno,&irxlam,&rxfq);
         if (nearest[10] == 'p')pfk();  /* select correct pcal via SBC */
         if (nearest[10] == 's')sfk();
         if (nearest[10] == 'i')ifk();
         rxno(irxno);
	 rxfq=rxfq+fdiff;
         cprintf("%s %d %5.2f \r\n",vori,irxlam,rxfq); 
         cprintf("OBSINP FSM1 1 \r\n"); /*oscillator unlock so we can change it*/ 
	 /* does this line contain extra OBSE commands following a ">"? */
         n1=locate('>',nearest,10)+1 ;
         if(n1 > 0){
            more:
            n2=strlen(nearest)-1;
            n3=locate('>',nearest,n1+1); 
            if(n2 >= n3)n2=n3 ;
            strcpy(obuf,&nearest[n1]);
            for (j=n1,nn=0; j<n2 ; j++,nn++)
              obuf[nn]=nearest[j];obuf[n2-n1]='\0';
            obuf[strlen(obuf)-1]='\0';
            cprintf("%s \r\n",obuf);
            n1=n3+1;
            if(n3 > 0 )goto more;
         }
         cprintf("%s \r\n",vork);
         strcpy(oband,band); 
       }  
     }
     inoise=strncmp(noise,"0",1);  /* set to 1 if noise is "1" */
     ipcal=strncmp(pcal,"0",1);  /* set to 1 if pcal is "1" */
     rxbits=rxbits & 0xFD;  /* clear existing tcal bit */
     if(inoise == 1 )rxbits=rxbits ^ 0x2; /* cal on */
     rxbits=rxbits & 0x2F;  /* clear existing pcal bits */
     if(ipcal == 1 )rxbits=rxbits ^ pcalmask;  /* phasecal on*/
     wrbits(devcua) ;  /* now write all those rx bits to SBC */
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
/* This is the bit stuff to send to the SBC in Mk3 IFD 
   This has, for bits 0->7, following effect:
   1=tcal,2=rx1,3=rx0,4=ifcal,5=rx2,6=pcal to sfk,7=sfk
*/
void ifk() { pcalmask=0x10;  } /* phase cal in IF */
void sfk() { pcalmask=0x40;  } /* set pcal to secondary focus*/
void pfk() { pcalmask=0x80;  } /* phase cal to prime focus*/
void rxno(i) /* in usual notation, rxnbits =0 for receiver 1 */
int i;
{
  rxbits=rxbits & 0xD3; /* clear existing  rxn bits */
  switch (i)
    {
    case 1: rxbits=rxbits ^ 0x00; break;
    case 2: rxbits=rxbits ^ 0x08; break;
    case 3: rxbits=rxbits ^ 0x04; break;
    case 4: rxbits=rxbits ^ 0x0C; break;
    case 5: rxbits=rxbits ^ 0x20; break;
    case 6: rxbits=rxbits ^ 0x28; break;
    case 7: rxbits=rxbits ^ 0x24; break;
    case 8: rxbits=rxbits ^ 0x2C; 
    }
}
void wrbits(unit)
/* this sends out 16 bits coded hex to boca port 1 for sbc in ifd */
int unit;
{
   int n1,b1,b2;
   char wbuf[100];
   b1=(rxbits >> 4) & 0xF; b2=rxbits & 0xF;
   sprintf(wbuf,">>RX=00%X%X\r\n",b1,b2);
//printf("DIAGNOSE:%s:\n",wbuf);
   n1=write(unit,wbuf,strlen(wbuf)); 
}
