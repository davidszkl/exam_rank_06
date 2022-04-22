#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct client {
	int	fd;
	int id;
} client;

client	clients[10000] = {0};
fd_set	read_s, write_s, memory_s;
int		sockfd;
int		id, max_fd;
char	buf[100000] = {0};
char	msg[100000] = {0};

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

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ||\
		listen(sockfd, 128) < 0)
		fatal();
	
	return sockfd;
}

void send_all(char *str, int from)
{
	for (int i = 0; i < id; i++)
		if (clients[i].fd > 0 && clients[i].fd != from)
			send(clients[i].fd, str, strlen(str), 0);
	bzero(msg, sizeof(msg));
}

void send_msg(char *str, int id, int from)
{
	int j = 0;
	for (unsigned int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
		{
			sprintf(msg, "client %d: %s\n", id, buf);
			send_all(msg, from);
			bzero(buf, sizeof(buf));
			j = 0;
			continue;
		}
		buf[j++] = str[i];
	}
}

void add_client()
{
	struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
	int i = 0;

	while (clients[i].fd != 0 && i < 10000)
		i++;
	if ((clients[i].fd = accept(sockfd, (struct sockaddr*)&clientaddr, &len)) < 0)
		fatal();
	clients[i].id = id++;
	max_fd++;
	bzero(msg, sizeof(msg));
	sprintf(msg, "server: client %d just arrived, fd = %d\n", clients[i].id, clients[i].fd);
	send_all(msg, clients[i].fd);
	FD_SET(clients[i].fd, &memory_s);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return write(STDERR_FILENO, "Wrong number of arguments\n", 27) && 1;
	
	sockfd = init_server(atoi(argv[1]));
	FD_ZERO(&read_s);
	FD_SET(sockfd, &memory_s);
	id = 0;
	max_fd = sockfd;

	while (1)
	{
		write_s = read_s = memory_s;
		if (select(max_fd + 1, &read_s, &write_s, NULL, NULL) < 0)
			fatal();
		if (FD_ISSET(sockfd, &read_s))
		{
			add_client();
			continue;
		}
		for (int i = 0; i < id; i++)
		{
			int fd = clients[i].fd;
			if (fd < sockfd || !FD_ISSET(fd, &read_s))
				continue;
			else
			{
				int rval = 100;
				while (rval == 100)
				{
					rval = recv(fd, buf + strlen(buf), 100, 0);
					if (rval < 0)
						fatal();
				}
				if (!rval)
				{
					sprintf(msg, "server: client %d just left\n", clients[i].id);
					send_all(msg, fd);
					if (close(fd) < 0)
						fatal();
					bzero(&clients[i], sizeof(client));
					FD_CLR(fd, &memory_s);
				}
				else
				{
					send_msg(buf, clients[i].id, fd);
					bzero(buf, sizeof(buf));
				}
			}
		}
	}
}