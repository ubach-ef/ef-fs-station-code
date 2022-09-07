/* Sample UDP client */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


int main(int argc, char**argv)
{
   int sockfd,n,iret;
   struct sockaddr_in servaddr;
   char sendline[1000];
   char recvline[1000];
   fd_set in_fdset;
   struct itimerval value;
   //int flags=0;
   //int fd_width=2;

   if (argc != 2)
   {
      printf("usage:  udpclient <IP address>\n");
      exit(1);
   }

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(22122);
   int on=1; 
   setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on, sizeof(on));
   FD_ZERO(&in_fdset);
   FD_SET(sockfd,&in_fdset);  
   value.it_value.tv_usec = 1000000;
   value.it_value.tv_sec = 0;

   while (fgets(sendline, 10000,stdin) != NULL)
   {
      sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      usleep(10000);
     n=select(FD_SETSIZE,&in_fdset,NULL,NULL,&value.it_value);
      if(n !=0)
     {
      n=recvfrom(sockfd,recvline,300,0,NULL,NULL);
      if(n <=0)printf("recv error n=%d\n",n);
     } else {printf("recv timeout\n"); }
      recvline[n]=0;
      fputs(recvline,stdout);
   }
}
