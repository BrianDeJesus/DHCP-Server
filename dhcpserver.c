/*
Brian DeJesus
extra credit Server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

pthread_t tid[3];

struct thread_args {
  char msg[50];
  char ack[50];
  char ip_offer[30];
  int socket;
};

#define MAXLINE	256

void sig_handler(int signal);
void read_and_reply(int socket_fd);
int found_key(char *buffer);
void *reply_ack(void *data);
void *reply_ip(void *data);
void *reply_msg(void *data);

const int backlog = 4;


int main(int argc, char *argv[])
{

    int	    listenfd, connfd;
    pid_t   childpid;
    int     clilen;
    struct  sockaddr_in cliaddr, servaddr;

    if (argc != 3) {// check command line arguments
    	printf("Usage: tcpserver <address> <port> \n");
    	return EXIT_FAILURE;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
    	perror("socket error");
    	return EXIT_FAILURE;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family        = AF_INET;
    servaddr.sin_addr.s_addr   = inet_addr(argv[1]);
    servaddr.sin_port          = htons(atoi(argv[2]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
	     perror("bind error");
        return EXIT_FAILURE;
    }

    if (listen(listenfd, backlog) == -1) {
    	perror("listen error");
    	return EXIT_FAILURE;
    }

    signal(SIGCHLD, sig_handler);

    while (1) {
      	clilen = sizeof(cliaddr);
      	if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0 ) {
      		if (errno == EINTR)
      			continue;
      		else {
      			perror("aceppt error");
      			return EXIT_FAILURE;
            close(connfd);
      		}
      	}
        read_and_reply(connfd);
    }

    return 0;
}

void read_and_reply(int socket_fd) {
	int	n, err;
	char	buf[80];
  struct thread_args *args = malloc(sizeof(struct thread_args));
  strcpy(args->msg, "DHCP offer! \n");
  strcpy(args->ack, "Server got the message! \n");
  strcpy(args->ip_offer, "192.168.1.123");
  args->socket = socket_fd;

  err = pthread_create(&(tid[0]), NULL, &reply_ack, (void *)args);
 if (err != 0)
     printf("\ncan't create thread 1 :[%s]", strerror(err));

 err = pthread_create(&(tid[1]), NULL, &reply_msg, (void *)args);
 if (err != 0)
     printf("\ncan't create thread 2 :[%s]", strerror(err));

err = pthread_create(&(tid[2]), NULL, &reply_ip, (void *)args);
    if (err != 0)
      printf("\ncan't create thread 2 :[%s]", strerror(err));


	while (1) {
      //   err = pthread_create(&(tid[0]), NULL, &reply_ack, (void *)args);
      //  if (err != 0)
      //      printf("\ncan't create thread 1 :[%s]", strerror(err));
      //
      //  err = pthread_create(&(tid[1]), NULL, &reply_msg, (void *)args);
      //  if (err != 0)
      //      printf("\ncan't create thread 2 :[%s]", strerror(err));
      //
      // err = pthread_create(&(tid[2]), NULL, &reply_ip, (void *)args);
      //     if (err != 0)
      //       printf("\ncan't create thread 2 :[%s]", strerror(err));

      bzero(&buf, sizeof(buf));
	    if ((n = read(socket_fd, buf, MAXLINE)) == 0) {
        perror("error reading from client to server");
  		    return;
        }

      if(found_key(buf) == 1) {
        printf("discover FOUND\n");
        pthread_join(tid[0], NULL);// offer message
        pthread_join(tid[1], NULL);// off
        pthread_join(tid[2], NULL);
      }
    //  write(socket_fd, ack, strlen(ack));
      printf("Message from client: %s \n" , buf);


     //pthread_join(tid[0], NULL);// offer message

	 }
}

void *reply_ack(void *data) { // Offer message function
  struct thread_args *args = data;
  write(args->socket, args->ack, strlen(args->ack));
  return NULL;
}

void *reply_ip(void *data) { // Offer message function
  struct thread_args *args = data;
  write(args->socket, args->ip_offer, strlen(args->ip_offer));
  return NULL;
}

void *reply_msg(void *data) { // Offer message function
  struct thread_args *args = data;
  write(args->socket, args->msg, strlen(args->msg));
  return NULL;
}


int found_key(char *buffer) { // Check for DHCP discover
  int i;
  char key[] = "DHCP discover";
  if(strncmp(buffer, key, strlen(key)) == 0){
      return 1;
    }
    return 0;
}

void sig_handler(int signal)
{
    pid_t    pid;
    int	     status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	   printf("child %d terminated \n", pid);

    return;

}
