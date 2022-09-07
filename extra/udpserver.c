         #include <stdio.h>
          #include <sys/socket.h>
          #include <arpa/inet.h>
          #include <stdlib.h>
          #include <string.h>
          #include <unistd.h>
          #include <netinet/in.h>

          #define BUFFSIZE 255
          void Die(char *mess) { perror(mess); exit(1); }
  int main(int argc, char *argv[]) {
            int sock;
	    int on;
            struct sockaddr_in echoserver;
//            struct sockaddr_in echoclient;
            char buffer[BUFFSIZE];
            unsigned int echolen, clientlen, serverlen;
            int received = 0;
            int nsent;

            if (argc != 2) {
              fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
              exit(1);
            }
                   /* Create the UDP socket */
          if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            printf("Failed to create socket\n");
          }
          on=1; 
          setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&on, sizeof(on));

          /* Construct the server sockaddr_in structure */
          memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
          echoserver.sin_family = AF_INET;                  /* Internet/IP */
          echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
          echoserver.sin_port = htons(atoi(argv[1]));       /* server port */

          /* Bind the socket */
          serverlen = sizeof(echoserver);
          if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
            Die("Failed to bind server socket");
          }
                  /* Run until cancelled */
            while (1) {
              /* Receive a message from the client */
//              clientlen = sizeof(echoclient);
              if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
                                       (struct sockaddr *) &echoserver,
                                       &serverlen)) < 0) {
                Die("Failed to receive message");
              }
              fprintf(stderr,
                      "Client connected: %s\n", inet_ntoa(echoserver.sin_addr));
                      buffer[received]='\0';
                      printf("MSG rcvd: %s\n",buffer);
              /* Send the  back to client */
              strcpy(buffer,"Thx, cookie tasted OK\n");
              nsent=strlen(buffer);
              sendto(sock, buffer, nsent, 0, (struct sockaddr *) &echoserver,
                         sizeof(echoserver));
                        printf("send back %d bytes\n",nsent);
            }
          }
          
