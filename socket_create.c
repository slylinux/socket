#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "socket_create.h"

unsigned short portbase=0;

int errexit(const char* format,...)
{
	va_list args;
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	exit(1);
}

int passive_sock(const char* service,const char* transport,int qlen,unsigned short port)
{
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s,type;

	memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(port);
	/* if(pse=getservbyname(service,transport)) */
	/* 	sin.sin_port=htons(ntohs((unsigned short)pse->s_port)+portbase); */
	/* else if((sin.sin_port=htons((unsigned short)atoi(service)))==0) */
	/* 	errexit("can't get \"%s\" service entry\n",service); */
	if((ppe=getprotobyname(transport))==0)
		errexit("can't get \"%s\" transport entry\n",transport);
	if(strcmp(transport,"udp")==0)
		type=SOCK_DGRAM;
	else
		type=SOCK_STREAM;
	s=socket(PF_INET,type,ppe->p_proto);
	if(s<0){
		errexit("can't create socket:%s\n",strerror(errno));
	}
	if(bind(s,(struct sockaddr*)&sin,sizeof(sin))<0){
		errexit("can't bind socket to %s port: %s\n",service,strerror(errno));
	}
	if(type==SOCK_STREAM&&listen(s,qlen)<0){
		errexit("can't listen on %s port: %s\n",service,strerror(errno));
	}

	return s;
}

int passiveTCP(const char* service,int qlen,unsigned short port)
{
	return passive_sock(service,"tcp",qlen,port);
}
