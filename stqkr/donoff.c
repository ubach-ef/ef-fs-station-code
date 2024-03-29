/* onoff snap command for dbbc */

#include <math.h>
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h>

#include "../../fs/include/dpi.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/fscom.h"         /* shared memory definition */
#include "../../fs/include/shm_addr.h"      /* shared memory pointer */

static char hex[]= "0123456789abcdef";
static char det[] = "dlu34567";
static float bw[ ]={0.0,0.125,0.250,0.50,1.0,2.0,4.0}; 
static float bw4[ ]={0.0,0.125,16.0,0.50,8.0,2.0,4.0};
static float bw_vlba[ ]={0.0625,0.125,0.25,0.5,1.0,2.0,4.0,8.0,16.0};
static float bw_lba[ ]={0.0625,0.125,0.25,0.5,1.0,2.0,4.0,8.0,16.0,32.0,64.0};
static char *lwhat[ ]={
"1l","2l","3l","4l","5l","6l","7l","8l","9l","al","bl","cl","dl","el",
"1u","2u","3u","4u","5u","6u","7u","8u","9u","au","bu","cu","du","eu",
"ia","ib","ic","id"};

float flux_val();

void donoff(command,itask,ip)
struct cmd_ds *command;                /* parsed command structure */
int itask;
int ip[5];                           /* ipc parameters */
{
      int ilast, ierr, ichold, i, count, j;
      int verr;
      char *ptr;
      struct onoff_cmd lcl;
      int it[6];
      char lsorna[sizeof(shm_addr->lsorna)+1];
      float epoch,corr,ssize;

      int onoff_dec();                 /* parsing utilities */
      char *arg_next();

      void onoffc_dis();

      void skd_run(), skd_par();      /* program scheduling utilities */

      ierr=0;

      if (command->equal != '=') {           /* run onoff */
	int set;
	if ( 1 == nsem_test("onoff") || 1 == nsem_test("fivpt")) {
	  ierr=-305;
	  goto error;
	}
	set=FALSE;
	for (i=0;i<MAX_ONOFF_DET;i++) {
	  set= set ||(lcl.itpis[i]!=0);
	}
	if(!set) {
	  ierr=-304;
	  goto error;
	}
	memcpy(&lcl,&shm_addr->onoff,sizeof(lcl));
	if(shm_addr->equip.rack==MK3||shm_addr->equip.rack==MK4||shm_addr->equip.rack==LBA4) {
	  float vcf,vcbw,extbw;
	  lcl.fwhm=-1.0;
	  for (i=0;i<14;i++)
	    if(lcl.itpis[i]!=0){
	      lcl.devices[i].ifchain=abs(shm_addr->ifp2vc[i]);
	      if(lcl.devices[i].ifchain<1||lcl.devices[i].ifchain>3)
		lcl.devices[i].ifchain=0;
	      lcl.devices[i].lwhat[0]=hex[i+1];
	      if(shm_addr->ITPIVC[i]==-1)
		lcl.devices[i].lwhat[1]='x';
	      else
		lcl.devices[i].lwhat[1]=det[shm_addr->ITPIVC[i]&0x7];
	      if(lcl.devices[i].ifchain!=0) {
		vcf=shm_addr->freqvc[i];
		if(shm_addr->ibwvc[i]==0)
		  vcbw=shm_addr->extbwvc[i];
		else if(shm_addr->equip.rack==MK3)
		  vcbw=bw[shm_addr->ibwvc[i]];
		else if(shm_addr->equip.rack==MK4)
		  vcbw=bw4[shm_addr->ibwvc[i]];
		switch (shm_addr->ITPIVC[i]&0x7) {
		case 0: /* dual */
		  vcf=vcf;
		  break;
		case 1: /* lsb */
		  vcf=vcf-vcbw*0.5;
		  break;
		case 2: /* usb */
		  vcf=vcf+vcbw*0.5;
		  break;
		default:
		  ierr=-301;
		  goto error;
		  break;
		}
		switch(shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
		case 1:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]+vcf;
		  break;
		case 2:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]-vcf;
		  break;
		default:
		  ierr=-302;
		  goto error;
		  break;
		}
	      } else {
		ierr=-303;
		goto error;
	      }
	    }
	  for (i=14;i<16;i++)
	    if(lcl.itpis[i]!=0) {
	      lcl.devices[i].ifchain=i-13;
	      lcl.devices[i].lwhat[0]='i';
	      lcl.devices[i].lwhat[1]=hex[i-13];
	      switch (shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
	      case 1:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]+(500.+100.)*0.5;
		break;
	      case 2:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]-(500.+100.)*0.5;
		break;
	      default:
		ierr=-302;
		goto error;
		break;
	      }
	    }
	  for (i=16;i<17;i++)
	    if(lcl.itpis[i]!=0) {
	      float upper;
	      lcl.devices[i].ifchain=i-13;
	      lcl.devices[i].lwhat[0]='i';
	      lcl.devices[i].lwhat[1]=hex[i-13];
	      if(shm_addr->imixif3==1)
		upper=400.0;
	      else
		upper=500.0;
	      switch (shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
	      case 1:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]+(upper+100.)*0.5;
		break;
	      case 2:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]-(upper+100.)*0.5;
		break;
	      default:
		ierr=-302;
		goto error;
		break;
	      }
	    }
	} else if(shm_addr->equip.rack==VLBA||shm_addr->equip.rack==VLBA4) {
	  for (i=0;i<MAX_BBC*2;i++) {
	    if(lcl.itpis[i]!=0) {
	      lcl.devices[i].ifchain=shm_addr->bbc[i%MAX_BBC].source+1;
	      if(lcl.devices[i].ifchain<1||lcl.devices[i].ifchain>4)
		lcl.devices[i].ifchain=0;
	      if(lcl.devices[i].ifchain!=0) {
		int bbc2freq();
		float freq, bbcbw;
		
		freq=bbc2freq(shm_addr->bbc[i%MAX_BBC].freq)/100.0;
		bbcbw=bw_vlba[shm_addr->bbc[i%MAX_BBC].bw[1-(i/MAX_BBC)]];
		if(i<MAX_BBC)
		  freq-=bbcbw*.5;
		else
		  freq+=bbcbw*.5;
		switch(shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
		case 1:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]+freq;
		  break;
		case 2:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]-freq;
		  break;
		default:
		  ierr=-302;
		  goto error;
		  break;
		}
	      } else {
		ierr=-306;
		goto error;
	      }
	    }
	  }
	  for (i=MAX_BBC*2;i<MAX_DET;i++) {
	    if(lcl.itpis[i]!=0) {
	      lcl.devices[i].ifchain=i-MAX_BBC*2+1;
	      switch (shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
	      case 1:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]+(500.+1000.)*0.5;
		break;
	      case 2:
		lcl.devices[i].center=
		  shm_addr->lo.lo[lcl.devices[i].ifchain-1]-(500.+1000.)*0.5;
		break;
	      default:
		ierr=-302;
		goto error;
		break;
	      }
	    }
	  }
	} else if(shm_addr->equip.rack==LBA) {
	  for (i=0;i<2*shm_addr->n_das;i++) {
	    if(lcl.itpis[i]!=0) {
	      lcl.devices[i].ifchain=shm_addr->das[i/2].ifp[i%2].source+1;
	      if(lcl.devices[i].ifchain<1||lcl.devices[i].ifchain>4)
		lcl.devices[i].ifchain=0;
	      if(lcl.devices[i].ifchain!=0) {
		float ifpf, ifpbw;
		
		ifpf=shm_addr->das[i/2].ifp[i%2].frequency;
		ifpbw=bw_lba[shm_addr->das[i/2].ifp[i%2].bandwidth];
		switch(shm_addr->das[i/2].ifp[i%2].filter_mode) {
		case _SCB:
		case _ACB:
		case _SC1:
		case _AC1:
		  /* Centre band - centred on IFP frequency */
		  break;
		case _DSB:
		case _DS2:
		  /* Double sideband - LSB or DSB, depends on setting */
		  if(shm_addr->das[i/2].ifp[i%2].ft.digout.setting)
		    ifpf-=ifpbw*.5;
		  else
		    ifpf+=ifpbw*.5;
		  break;
		case _DS4:
		  /* Outer double sideband - LSB or DSB, depends on setting */
		  if(shm_addr->das[i/2].ifp[i%2].ft.digout.setting)
		    ifpf-=ifpbw*1.5;	/* Valid only for 8MHz BW */
		  else
		    ifpf+=ifpbw*1.5;	/* Valid only for 8MHz BW */
		  break;
		case _DS6:
		  /* Extreme double sideband - LSB or DSB, depends on setting */
		  if(shm_addr->das[i/2].ifp[i%2].ft.digout.setting)
		    ifpf-=ifpbw*2.5;	/* Valid only for 8MHz BW */
		  else
		    ifpf+=ifpbw*2.5;	/* Valid only for 8MHz BW */
		  break;
		default:
		  ierr=-301;
		  goto error;
		  break;
		}
		switch(shm_addr->lo.sideband[lcl.devices[i].ifchain-1]) {
		case 1:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]+ifpf;
		  break;
		case 2:
		  lcl.devices[i].center=
		    shm_addr->lo.lo[lcl.devices[i].ifchain-1]-ifpf;
		  break;
		default:
		  ierr=-302;
		  goto error;
		  break;
		}
	      } else {
		ierr=-306;
		goto error;
	      }
	    }
	  }
	}
	/* user devices */
	for (i=MAX_DET;i<MAX_ONOFF_DET;i++)
	    if(lcl.itpis[i]!=0) {
	      lcl.devices[i].ifchain=i-MAX_DET+1;
	      lcl.devices[i].lwhat[0]='u';
	      lcl.devices[i].lwhat[1]=hex[i-MAX_DET+1];
	      lcl.devices[i].center=
		shm_addr->user_device.center[lcl.devices[i].ifchain-1];
	      switch(shm_addr->user_device.sideband[lcl.devices[i].ifchain-1]) {
	      case 1:
		lcl.devices[i].center=
		  shm_addr->user_device.lo[lcl.devices[i].ifchain-1]
		  +lcl.devices[i].center;
		break;
	      case 2:
		lcl.devices[i].center=
		  shm_addr->user_device.lo[lcl.devices[i].ifchain-1]
		  -lcl.devices[i].center;
		break;
	      default:
/*
		printf(" user device sideband %d ifchain %d \n",
		      shm_addr->user_device.sideband[lcl.devices[i].ifchain-1],
		      lcl.devices[i].ifchain);
*/
		ierr=-302;
		goto error;
		break;
	      }
	    }

	memcpy(lsorna,shm_addr->lsorna,sizeof(lsorna)-1);
	lsorna[sizeof(lsorna)-1]=0;
	for(j=0;j<sizeof(lsorna)-1;j++)
	  if(lsorna[j]==' ') {
	    lsorna[j]=0;
	    break;
	  }
	rte_time(it,it+5);
	epoch=((float) it[5])+((float) it[4])/366.;
	lcl.ssize=0.0;
	for(i=0;i<MAX_ONOFF_DET;i++) {
	  if(lcl.itpis[i]!=0) {
	    int pol,ifchain;
	    ifchain=lcl.devices[i].ifchain;
	    if(1 <= ifchain && ifchain <= 4) {
	      pol=shm_addr->lo.pol[ifchain-1];
	    } else if (5 <= ifchain && ifchain <= 6) {
	      pol=shm_addr->user_device.pol[ifchain-1];
	    } 
	    switch(pol) {
	    case 1:
	      lcl.devices[i].pol='r';
	      break;
	    case 2:
	      lcl.devices[i].pol='l';
	      break;
	    default:
	      lcl.devices[i].pol='u';
	      break;
	    }
	    get_gain_par(lcl.devices[i].ifchain,
			 lcl.devices[i].center,
			 &lcl.devices[i].fwhm,
			 &lcl.devices[i].dpfu,
			 &lcl.devices[i].gain,
			 &lcl.devices[i].tcal);
		   
	    lcl.fwhm=lcl.devices[i].fwhm>lcl.fwhm
	      ?lcl.devices[i].fwhm : lcl.fwhm;
	    
	    lcl.devices[i].flux=flux_val(lsorna,&shm_addr->flux,
					 lcl.devices[i].center,
					 epoch,
					 lcl.devices[i].fwhm,
					 &lcl.devices[i].corr,&ssize);
	    lcl.ssize=lcl.ssize>ssize?lcl.ssize:ssize;
	    if(lcl.devices[i].corr>=1.2) {
	      memcpy(ip+3,"q1",2);
	      logita(NULL,-307,ip+3,lcl.devices[i].lwhat);
	    }
	  }
	}

	lcl.setup=TRUE;
	lcl.stop_request=0;
	memcpy(&shm_addr->onoff,&lcl,sizeof(lcl));
	skd_run("dbbconoff",'n',ip);
	ip[0]=ip[1]=ip[2]=0;
	return;
      } else if (command->argv[0]==NULL)
	goto parse;  /* simple equals */
      else if (command->argv[1]==NULL) {/* special cases */
        if (*command->argv[0]=='?') {
          onoff_dis(command,ip);
	  return;
	} else if(0==strcmp(command->argv[0],"stop")){
	  shm_addr->onoff.stop_request=1;
	  skd_run("dbbconoff",'w',ip);
	  ip[0]=ip[1]=ip[2]=0;
          return;
	}
      }
/* if we get this far it is a set-up command so parse it */

parse:
      ilast=0;                                      /* last argv examined */

      memcpy(&lcl,&shm_addr->onoff,sizeof(lcl));
      count=1;
      while( count>= 0) {
        ptr=arg_next(command,&ilast);
        ierr=onoff_dec(&lcl,&count, ptr);
        if(ierr !=0 ) goto error;
      }

/* all parameters parsed okay, update common */

      memcpy(&shm_addr->onoff,&lcl,sizeof(lcl));

      ip[0]=ip[1]=ip[2]=0;

      onoff_dis(command,ip);
      return;

error:
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"q1",2);
      return;
}
