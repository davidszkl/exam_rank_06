#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <netdb.h>

// void get_local()
// {
// 	unsigned int a = inet_addr("127.0.0.1");
// 	printf("%u\n", ntohl(a));
// }

typedef struct client {
	int	fd;
	int id;
} client;

typedef struct server {
	struct sockaddr_in	sockfd;
	fd_set				rset;
	fd_set				read_s;
	fd_set				write_s;
	client				clients[10000];
	int					max_fd;
	int					id;
	int					count;
} s_server;

void fatal()
{
	write(STDERR_FILENO, "Fatal error\n", 13);
	exit(1);
}

int init_server(unsigned int port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;
	
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");//ntohl(2130706433);
	server_addr.sin_port		= htons(port);

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)	
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;

	if (listen(sockfd, 128) < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;
	
	return sockfd;
}

void send_all(char *str, s_server* ptr, int from)
{
	printf("MESSAGE = %s", str);
	for (int i = 0; i <= ptr->count + 1; i++)
	{
		if (ptr->clients[i].fd > 0 && ptr->clients[i].fd != from)
		{
			printf("TESTING = %d\n", ptr->clients[i].fd);
			send(ptr->clients[i].fd, str, strlen(str), 0);
		}
	}
}

void send_msg(char *str, s_server* ptr, int from)
{
	char send_buff[100000] = {0};
	char other_buff[10000] = {0};
	int j = 0;
	for (unsigned int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
		{
			sprintf(send_buff, "client %d: %s\n", ptr->clients[from].id, other_buff);
			send_all(send_buff, ptr, from);
			bzero(send_buff, sizeof(send_buff));
			bzero(other_buff, sizeof(other_buff));
			j = 0;
			continue;
		}
		other_buff[j++] = str[i];
	}
}

void add_client(int fd, s_server* ptr)
{
	struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
	char msg[1000];

	printf("adding client...\n");
	for (int i = 0; i <= ptr->count + 1; i++)
	{
		if (ptr->clients[i].fd == 0)
		{
			printf("client[%d].fd = %d\n", i, ptr->clients[i].fd);
			ptr->clients[i].fd = accept(fd, (struct sockaddr*)&clientaddr, &len);
			if (ptr->clients[i].fd < 0)
				fatal();
			ptr->clients[i].id = ptr->id++;
			ptr->count++;
			ptr->max_fd++;
			sprintf(msg, "server: client %d just arrived\n", ptr->clients[i].id);
			send_all(msg, ptr, ptr->clients[i].fd);
			FD_SET(ptr->clients[i].fd, &ptr->rset);
			break;
		}
	}
}

void my_log(s_server *ptr)
{
	printf("clients = %d\nmax_fd = %d\ncurr_id = %d\n", ptr->count, ptr->max_fd, ptr->id);
	printf("first 5 clients:\n");
	for (int i = 0; i < 5; i++)
	{
		if (ptr->clients[i].fd <= 0)
			break;
		printf("client %d: fd = %d\n", ptr->clients[i].id, ptr->clients[i].fd);
	}
}

void show_set(fd_set set)
{
	printf("%d\n", set.fds_bits[0]);
}

int main(int argc, char **argv)
{
	//if (argc != 2)
		//return write(STDERR_FILENO, "Wrong number of arguments\n", 27) && 1;

	//int sockfd = init_server(atoi(argv[1]));
	int sockfd = init_server(4242);
	(void)argc;
	(void)argv;

	s_server server;
	FD_ZERO(&server.rset);
	FD_SET(sockfd, &server.rset);
	bzero(server.clients, sizeof(server.clients));
	server.max_fd	= sockfd;
	server.id		= 0;
	server.count	= 0;
	server.clients[0].fd = sockfd;

	char buf[100000] = {0};
	char msg[100000] = {0};

	while (1)
	{
		server.read_s = server.write_s = server.rset;
		printf("selecting...\n");
		if (select(server.max_fd + 1, &server.read_s, &server.write_s, NULL, NULL) < 0)
			continue;

		my_log(&server);
		for (int i = 0; i <= server.count + 1; i++)
		{
			int fd = server.clients[i].fd;
			if (FD_ISSET(fd, &server.read_s))
			{
				if (fd == sockfd)
				{
					add_client(fd, &server);
					continue;
				}
				int rval = 1;
				memset(buf, 0,sizeof(buf));
				while (rval > 0)
				{
					rval = recv(fd, buf + strlen(buf), 1000, 0);
					if (rval < 0)
						fatal();
				}
				if (!rval)
				{
					sprintf(msg, "server: client %d just left\n", server.clients[i].id);
					send_all(msg, &server, fd);
					if (close(fd) < 0)
						fatal();
					memset(&server.clients[i], 0, sizeof(client));
					FD_CLR(fd, &server.rset);
				}
				else
				{
					send_msg(msg, &server, i);
				}
				
			}
		}
	}
	return 0;
}