#include <stdio.h>
#include <sys/types.h>
#include "../include/stparams.h" 
#include "../include/stcom.h"   
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"  
#include "../../fs/include/fs_types.h" 
#include "../../fs/include/shm_addr.h" 
#include "../../fs/include/fscom.h"  
#define MAXLEN 4095
int portopen_();
int portwrite_();
int portread_();
void eraseAllBlanks();
/* initialise hp counter (here 5328A), read value from it*/
/* send to double gpsdiff in stcom*/

extern struct stcom *st;
main(int argc, char *argv[])
{
  int ttynum;      /* read from appropriate com port as number */
  int baud_rate;    /* baudrate in Kbaud */
  int parity;       /* parity bit */
  int bits;         /* bits */
  int stop;         /* stop bits */
  int buffsize;     /* read and write buffer size. */
  
  /* Local variables. */
  char terminal[40];    /* name of terminal device. */
  char string[5];       /* place holder for int to char */
  int open_err;         /* terminal error on open. */
  char buff[MAXLEN];
  int len;
  int termch, err, count, to;
  int i,iii;
  int read_errors=0;
  int write_errors=0;
  long idiff;
  double tdiff,tdiffout;
  
  setup_st();
  /* presenting a time stamp */

      baud_rate = 115200;
      parity = 1;
      bits = 7;
      stop = 1;
      buffsize = 256;

  /* Create device name. */
    strcpy(terminal, "/dev/ttyUSB0");

  /* OPEN device and terminal. */
  len = strlen(terminal);
  open_err = portopen_(&ttynum, terminal, &len,
		   &baud_rate, &parity, &bits, &stop);

  termch=0x0a; i=100; to=200; /*lf term, max 100 char , 200 centisec timeout*/
  //just for check: ask USB/GPIB adapter if it is OK
  //set up USB to GPIB adapter
  //can leave out the ++ver and readback of result...
      strcpy(buff,"++ver\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      err = portread_(&ttynum, buff, &count, &i, &termch, &to);
//      printf("USB-GPIB adapter version=%s",buff);

      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      strcpy(buff,"++mode 1\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      strcpy(buff,"++addr 3\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      strcpy(buff,"++auto 1\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
  //now counter settings
      strcpy(buff,"R\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      sleep(1); // yawn...
/* original */
//      strcpy(buff,"PF8G0S4T\n"); len=strlen(buff);
/* added level control for the inputs */
      strcpy(buff,"PF8G0S4A3A+010B3B+010T\n"); len=strlen(buff);
/* reduced frequency resolution to avoid overflow */
/* see page 59 of HP5328B.pdf */
//      strcpy(buff,"PF8G1S4A3A+010B3B+010T\n"); len=strlen(buff);
      if (portwrite_(&ttynum,buff,&len) <0)printf("portwrite err_n") ;
      sleep(2); // yawn...wait for measurement to be done
      /* read back from port */
/* do it 2 times until reasonable value*/
// for(iii=0; iii<2; iii++){
   err = portread_(&ttynum, buff, &count, &i, &termch, &to);
   buff[count] = '\0';
   len=strlen(buff);
   eraseAllBlanks(buff);
  sscanf(buff,"%le",&tdiff);
//  tdiff=atof(buff);
//  printf("length of reply=%d reply=%s gpsdiff=%e\n",len,buff,tdiff);
   tdiffout=tdiff;
   if ((tdiff > 5e-4)) tdiffout=tdiff-1; /*eff problem*/
  printf("%e\n",tdiffout);
//}
  stm_addr->gpsdiff=tdiff;
  portclose_(&ttynum);
  return 0;
}
/* ******************************* */
void eraseAllBlanks(char *src)
{
	char *dst = src;

	while (*src != 0) {
		if (*src != ' ') {
			*dst++ = *src; // copy
		}
		src++;
	}
	*dst = 0;
}
