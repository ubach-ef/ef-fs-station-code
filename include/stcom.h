/* stcom.h - dummy
 */

typedef struct stcom {
  int dummy;              /* just a dummy */
  int ieffcal;            /* 0 for frontend cal off, 1 for cal on */
  int ieffpcal;           /* 0 for phase cal off, 1 for pcal on */
  long ieffrx;            /* what receiver (now lo frequency)*/
  int rx_bits;            /* these are the 16bits sent to the SBC ,
                             top bit is cal offon */
  int antenna_sentoff;    /* if =1, antenna has just got new source command or azel offset, 
			     set to 0 by antrcv as soon as antenna starts to move and reports slewing*/
  int raw_ionsor;         /* =0 if ant off, 1 if on: cook to get ionsor special for on/off*/
  int ant_state ;          /*like ionsor in fscom, but more states: 
                            0=off and likely to remain off  (ionsor=0)
			    1=just told to move             (ionsor=0)
			    2=told to move and started motion (ionsor=0)
			    3= reached source but may still wobble off again (ionsor=1 if
			      in normal VLBI mode but but 0 if on/off or fivpt)
			    4=stable on source (ionsor=1)*/
  int ant_scanning;       /*from vaxtime, -ve if waiting, + if scanning (scanmsec)*/
  long ant_rfcentre;      /* from vaxtime broadcast: centre freq reported back from antenna,
			     should agree with value in rxlist table , if not, try to resend..*/
  double fsynth1,fsynth2; /*front end synthesizer(ULO) and 2nd LO synth in MHz*/
  int ant_is_listening;   /*controlled and read by rxvt, =1 if antenna is receiving commands from 
			    VME and mk4fs, =0 if F1 pressed to disconnect from antenna*/
  char ant_command[150];  /* source,freq request string received via rpc from VME or this machine,
			     this replaces direct socket connection to eff_rxvt used previously*/
  double gpsdiff; /*to hold result from gps counter*/
} Stcom;

