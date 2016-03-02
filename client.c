#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_LINE 512

int main(int argc,char *argv[])
{
	int port = 0;
	int sockfd;
	struct sockaddr_in server_addr;
	char buf[MAX_LINE+1];
	
	if(argc < 3)
		return;
	
	port = atoi(argv[2]);

	sockfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

	connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));

	while(1)
	{
		buf[0] = 0;
		if(fgets(buf,MAX_LINE+1,stdin) != NULL)
		{
			write(sockfd,buf,strlen(buf));
		}
	}
	
	return 0;
	
}

