#define ADRESSENFILE   "sock_adr.dat"
 
#ifndef  INVALID_SOCKET
#define INVALID_SOCKET    -1
#endif

#ifndef  SOCKET_ERROR
#define SOCKET_ERROR    -1
#endif

#ifndef BOOL
#define BOOL short
#endif

#define RICHTIG   (BOOL)1
#define FALSCH    (BOOL)0
#ifndef TRUE
#define TRUE      (BOOL)1  
#endif
#ifndef FALSE
#define FALSE     (BOOL)0
#endif  
#ifndef  NULL
#define NULL	   0
#endif
#define FOREVER   for(;;) {
#define ENDFOREVER   }

#define FATAL					(long)-1
#define NORMAL				(long)1
#define INFORM				(long)0
#define ADDSYSERR		(long)-999    

#define SOCK_ADDR_SIZE		(sizeof (struct sockaddr_in))
#define MAX_QUEUED_CONNECTIONS	(4)

/* char	*pname;  */

/* structure for requests from client to server */
/*
#define MSG_SIZE		(1024)
#define REPLY_MSG_SIZE		(1024)
struct request
	{
		int reply;
		char message[MSG_SIZE];
	};
*/

