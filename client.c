#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <netdb.h>

void func(int sockfd)
{
    char buff[100000];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
		read(STDIN_FILENO, buff, 100);
		printf("entered the string:\n%s", buff);
        write(sockfd, buff, sizeof(buff));
		printf("write  done\n");
        bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
        printf("%s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;

	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(4242);

	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;
	
	func(sockfd);
	close(sockfd);
	return 0;
}