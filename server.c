#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/epoll.h>



#define MAX_EPOLL_EVENTS 5
#define MAX_LINE 512

typedef void* (*PF_EPOLL_CALLBACK)(int fd,unsigned int events);

typedef struct{
	int fd;
	PF_EPOLL_CALLBACK pfCallback;
}MY_EPOLL_DATA;



int epollfd = -1;

void* listenfd_callback(int fd,unsigned int events);
void* clientfd_callback(int fd,unsigned int events);

int epoll_add_listenfd(int fd)
{
	struct epoll_event event;

	MY_EPOLL_DATA *myEpollData = malloc(sizeof(MY_EPOLL_DATA));
	bzero(myEpollData,sizeof(myEpollData));

	myEpollData->fd = fd;
	myEpollData->pfCallback = listenfd_callback;
	event.events = EPOLLIN | EPOLLERR | EPOLLHUP ;
	event.data.ptr = myEpollData;

	return epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
}

int epoll_add_clientfd(int fd)
{
	struct epoll_event event;

	MY_EPOLL_DATA *myEpollData = malloc(sizeof(MY_EPOLL_DATA));
	bzero(myEpollData,sizeof(myEpollData));

	myEpollData->fd = fd;
	myEpollData->pfCallback = clientfd_callback;
	event.events = EPOLLIN | EPOLLERR | EPOLLHUP ;
	event.data.ptr = myEpollData;

	return epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
}

void* listenfd_callback(int fd,unsigned int events)
{
	socklen_t clilen;
	struct sockaddr_in cliaddr;

	clilen = sizeof(cliaddr);
	int clifd = accept(fd,(struct sockaddr *)&cliaddr,&clilen);
	if(clifd > 0)
		printf("accept fd %d\n",clifd);

	epoll_add_clientfd(clifd);

	return NULL;
}

void* clientfd_callback(int fd,unsigned int events)
{
	ssize_t n;
	char buf[MAX_LINE+1];

	if(( n = read(fd,buf,MAX_LINE))>0)
	{
		buf[n] = 0;
		printf("read from fd [%d] [%s]\n",fd,buf);
	}

	return NULL;
}


int main(int argc,char *argv[])
{
	struct sockaddr_in server_addr;
	struct epoll_event events[MAX_EPOLL_EVENTS];
	int listenfd;
	int i,nums;
	MY_EPOLL_DATA *myEpollData;
	
	if(argc < 2)
		return;

	epollfd = epoll_create(MAX_EPOLL_EVENTS);
	
	listenfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	bind(listenfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
	listen(listenfd,5);
	epoll_add_listenfd(listenfd);
	
	for(;;)
	{
		nums = epoll_wait(epollfd,events,MAX_EPOLL_EVENTS,-1);
		printf("epoll wait nums[%d]\n",nums);
		for(i = 0;i < nums;i++)
		{
			myEpollData = (MY_EPOLL_DATA *)events[i].data.ptr;
			myEpollData->pfCallback(myEpollData->fd,events[i].events);
		}

		printf("sleeping\n");
		sleep(20);
		printf("sleeping end\n");
	}

	return 0;
	
}
