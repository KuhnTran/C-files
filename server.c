
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>

 // the port client will be connecting to
#define PORT 8192
#define MYPORT PORT
#define MAXDATASIZE 300
#define MAX_CLIENTS 100
/* how many pending connections queue will hold */
#define BACKLOG 10

static unsigned int cli_count = 0;
static int uid = 10;

typedef struct client_t
{
	struct sockaddr_in addr;	/* Client remote address */
	int connfd;			/* Connection file descriptor */
	int uid;			/* Client unique identifier */
	char name[32];			/* Client name */
} client_t;

static struct client_t *clients[MAX_CLIENTS];

void queue_add(struct client_t *cl)
{
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!clients[i])
		{
			clients[i] = cl;
			return;
		}
	}
}

void send_message(char *s, int uid)
{
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
		if(clients[i] &&  (clients[i]->uid != uid)) 
			write(clients[i]->connfd, s, strlen(s));
}

void send_message_all(char *s)
{
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
		if(clients[i]) write(clients[i]->connfd,s,strlen(s));
}

void send_message_self(char *s, int connfd)
{
	write(connfd, s, strlen(s));
}

void strip_newline(char *s)
{
	while(*s != '\0')
	{
		if (*s == '\r' || *s == '\n')
			*s = '\0';
		++s;
	}
}

void *handle_client(void *arg)
{
	char buff_out[1024];
	char buff_in[1024];
	int rlen;

	struct client_t *cli = (client_t *)arg;
	sprintf(buff_out, "<<JOIN, HELLO %s\r\n", cli->name);
	send_message_all(buff_out);
	
	while(1)
	{
		for (int i = 0; i < 1024; i++)
		{
			buff_in[i] = '\0';
			buff_out[i] = '\0';
		}		
		rlen = read(cli->connfd, buff_in, sizeof(buff_in) -1);
		
		buff_in[rlen] = '\0';
		buff_out[0] = '\0';
		strip_newline(buff_in);

		if (strcmp(buff_in, "QUIT") == 0)
		{
			printf("got a quit signal\n");
			char *quitSignal = "QUIT";
			send_message_self(quitSignal, cli->connfd);
			break;
		}
		
		if(!strlen(buff_in)) continue;

		sprintf(buff_out, "[%s] %s\r\n", cli->name, buff_in);
		send_message(buff_out, cli->uid);
	
	}
	close(cli->connfd);
	sprintf(buff_out, "<<LEAVE, BYE %s\r\n", cli->name);
	send_message_all(buff_out);
	
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());
}
	
void handle(int field)
{
	char buffer[1024];
	int nbytes;
	nbytes = read(field, buffer, 1024);
	send_message_all(buffer);
}	

int main()
{
    /* listen on sock_fd, new connection on new_fd */
    int sockfd, new_fd;
    /* my address information, address where I run this program */
    struct sockaddr_in my_addr;
    /* remote address information */
    struct sockaddr_in their_addr;
    int sin_size;
    pthread_t tid;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
      perror("socket() error lol!");
      exit(1);
    }
    else
      printf("socket() is OK...\n");
     
    /* host byte order */
    my_addr.sin_family = AF_INET;
    /* short, network byte order */
    my_addr.sin_port = htons(MYPORT);
    /* auto-fill with my IP */
    my_addr.sin_addr.s_addr = INADDR_ANY;
     
    /* zero the rest of the struct */
    memset(&(my_addr.sin_zero), 0, 8);
     
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
      perror("bind() error lol!");
      exit(1);
    }
    else
      printf("bind() is OK...\n");
     
    if(listen(sockfd, BACKLOG) == -1)
    {
      perror("listen() error lol!");
      exit(1);
    }
    else
      printf("listen() is OK...\n");

    fd_set active_fd_set, read_fd_set;
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);
    /* ...other codes to read the received data... */
    while (1)
    {
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
					 (socklen_t *)&sin_size);
		if(new_fd == -1)
		    perror("accept() error lol!");
		else
		    printf("accept() is OK...\n");
	     
		++cli_count;
		struct client_t *cli = 
			(struct client_t *)malloc(sizeof(client_t));
		cli->addr = their_addr;
		cli->connfd = new_fd;
		cli->uid = uid++;
		sprintf(cli->name, "%d", cli->uid);
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

    }
    
    close(sockfd);
    return 0;
}
