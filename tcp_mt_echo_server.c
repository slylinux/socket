#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>

#include "socket_create.h"

#define QLEN 32
#define BUFSIZE 4096
#define INTERVAL 5
int errexit(const char* format,...);

struct {
	pthread_mutex_t st_mutex;
	unsigned int st_concount;
	unsigned int st_contotal;
	unsigned int st_contime;
	unsigned int st_bytecount;
}stats;

int tcp_echod(int fd)
{
	time_t start;
	char buf[BUFSIZE];
	int cc;
	start=time(0);
	pthread_mutex_lock(&stats.st_mutex);
	stats.st_concount++;
	pthread_mutex_unlock(&stats.st_mutex);

	while(cc=read(fd,buf,BUFSIZE)){
		if(cc<0){
			errexit("echo read %s\n",strerror(errno));
		}
		if(write(fd,buf,cc)<0){
			errexit("echo write %s\n",strerror(errno));
		}
		pthread_mutex_lock(&stats.st_mutex);
		stats.st_bytecount+=cc;
		pthread_mutex_unlock(&stats.st_mutex);
	}

	close(fd);
	pthread_mutex_lock(&stats.st_mutex);
	stats.st_contime+=time(0)-start;
	stats.st_concount--;
	stats.st_contotal++;
	pthread_mutex_unlock(&stats.st_mutex);
	return 0;
}


int main(int argc,char *argv[])
{
	pthread_t th;
	pthread_attr_t ta;
	char* service="echo";
	struct sockaddr_in fsin;
	unsigned int alen;
	unsigned short port;
	int msock;
	int ssock;
	if(argc!=2)
		errexit("usage: app [port]\n");

	switch(argc){
	case 1:
		break;
	case 2:
		port=atoi(argv[1]);
		break;
	default:
		break;
	}

	msock=passiveTCP(service,QLEN,port);
	pthread_attr_init(&ta);
	pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&stats.st_mutex,0);

	while(1){
		alen=sizeof(fsin);
		ssock=accept(msock,(struct sockaddr*)&fsin,&alen);
		if(ssock<0){
			if(errno==EINTR)
				continue;
			errexit("accept : %s\n",strerror(errno));
		}
		if(pthread_create(&th,&ta,(void * (*)(void*))tcp_echod,(void*)ssock)<0)
			errexit("pthread create:%s\n",strerror(errno));
	}
}
