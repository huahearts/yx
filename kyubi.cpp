/*************************************************************************
	> File Name: kyubi.cpp
	> Author: alan
	> Mail: alan@163.com 
	> Created Time: 2021年07月18日 星期日 03时07分10秒
 ************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

int main()
{
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1)
	{
	    printf("create socket error");
		return -1;
	}

	int on = 1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on));
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEPORT,(char*)&on,sizeof(on));

	int oldSockFlag = fcntl(listenfd,F_GETFL,0);
	int newSockFlag = oldSockFlag | O_NONBLOCK;
	if(fcntl(listenfd,F_SETFL,newSockFlag) == -1)
	{
	    close(listenfd);
		printf("set nonblock error");
		return -1;
	}

	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(3000);
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr*)&bindaddr,sizeof(bindaddr)) == -1)
	{
		printf("bind error");
		close(listenfd);
		return -1;
	}

	if(listen(listenfd,SOMAXCONN) == -1)
	{
		printf("listen error");
		close(listenfd);
		return -1;
	}

	int epollfd = epoll_create(1);
	if(epollfd == -1)
	{
		printf("create epoll error");
		close(listenfd);
		return -1;
	}

	epoll_event listen_fd_event;
	listen_fd_event.data.fd = listenfd;
	listen_fd_event.events = EPOLLIN;

	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&listen_fd_event) == -1)
	{
		printf("epoll add event error");
		close(listenfd);
		return -1;
	}

	int n;
	while(true)
	{
		epoll_event events[1024];
		n = epoll_wait(epollfd,events,1024,1000);
		if (n < 0)
		{
			if(errno == EINTR)
				continue;
			break;
		}
		else if(n == 0)
		{
			continue;
		}

		for(size_t i = 0;i < n;i++)
		{
			if(events[i].events & EPOLLIN)
			{
				if(events[i].data.fd == listenfd)
				{
				    struct sockaddr_in clientaddr;
					socklen_t len = sizeof(clientaddr);
					int clientfd = accept(listenfd,(sockaddr*)&clientaddr,&len);
					if(clientfd != -1)
					{
						
						int oldSockFlag = fcntl(clientfd,F_GETFL,0);
						int newSockFlag = oldSockFlag | O_NONBLOCK;
						if(fcntl(clientfd,F_SETFL,newSockFlag) == -1)
						{
							close(clientfd);
							printf("set nonblock error");
						}
						else
						{
						    epoll_event client_fd_event;
							client_fd_event.data.fd = clientfd;
							client_fd_event.events = EPOLLIN;
							client_fd_event.events |= EPOLLONESHOT;
							if(epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&client_fd_event) != -1)
							{
							    printf("epoll add clientfd");
							}
							else
							{
								printf("add client fd to epollfd error");
								close(clientfd);
							}
						}
					}
				}
				else
				{
					printf("clientfd :%d,recvdata.\n",events[i].data.fd);
					char ch;
					int m = recv(events[i].data.fd,&ch,1,0);
					if(m == 0)
					{
						if(epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,NULL) != -1)
						{
						    printf("client disconnected,clientfd:%d\n",events[i].data.fd);
						}
						close(events[i].data.fd);

					}
					else if(m < 0)
					{
						if(errno != EWOULDBLOCK && errno != EINTR)
						{
						    if(epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,NULL) != -1)
							{
								printf("client disconnected,clientfd:%d\n",events[i].data.fd);
							}
							close(events[i].data.fd);
						}
					}
					else
					{
					    printf("recv from client: %d,data:%c\n",events[i].data.fd,ch);
					}
				}
			}
			else if(events[i].events & POLLERR)
			{
			//TODO
			}
		}
	}
	close(listenfd);
	return 0;
}
