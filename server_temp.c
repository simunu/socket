/*********************************************************************************
e*e     Copyright:  (C) 2022 Znu Lijun
 *                  All rights reserved.
 *
 *       Filename:  server_temp1.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(10/26/22)
 *         Author:  Zhu Lijun <3262465970@qq.com>
 *      ChangeLog:  1, Release initial version on "10/26/22 07:54:49"
 *                 
 ********************************************************************************/
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <errno.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <stdlib.h>
 #include <getopt.h>

 //#define LISTEN_PORT   8889
 #define BACKLOG       13
  
/*void printf_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("-p(--port): sepcify server port \n");
	printf("-h(--help): printf this help information \n");
}
*/

 int main(int argc,char **argv)
{
	int listen_fd = -1;
	int client_fd = -1;
	int rv = -1;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	socklen_t    cli_addr_len = sizeof(struct sockaddr);
	char         buf[1024];
//	int          LISTEN_PORT = 8887;				 
	int   		 port = 8887;
//	int			 ch;
//	int			 index;
/*	struct option opts[] = {
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc,argv,"p:h",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'p':
				port = atoi(optarg);
				break;
			case'h':
				printf_usage(argv[0]);
				return 0;
		}
	}

	while(argc < 2)
	{
		printf("please input %s [port] \n",argv[0]);
		return -1;
	}

*/				 
	if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
	{
		printf("create socket failure:%s\n",strerror(errno));
		return -1;
	}
	printf("socket create fd[%d]\n",listen_fd);
			     
	memset(&serv_addr,0,sizeof(serv_addr));
	memset(&cli_addr,0,sizeof(cli_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((bind(listen_fd,(struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0 )
	{
		printf("bind socket failure:%s\n",strerror(errno));
		return -2;
	}
	printf("socket[%d] bind on port[%d] for all IP address ok\n",listen_fd,port);

	if(listen(listen_fd,100) <0 )
	{
		printf("listen failure:%s\n",strerror(errno));
		return -3;
	}
	printf("connect server successfully\n");

	listen(listen_fd,BACKLOG);
						  
	while(1)
	{
		client_fd = accept(listen_fd,(struct sockaddr *)&cli_addr,&cli_addr_len);
		if(client_fd < 0 )
		{
			printf("accept new socket failure:%s\n",strerror(errno));
			return -4;
		}									     
											     
		memset(buf,0,sizeof(buf));
		if((read(client_fd,buf,sizeof(buf))) < 0)
		{
			printf("read data from client socket[%d] failure:%s\n",client_fd,strerror(errno));
			close(client_fd);
			rv = -1;
		}
		printf("The temperature('C) is :%s\n",buf);

		if((write(client_fd,buf,strlen(buf))) <0 )
		{

			printf("write %d bytes data back to client[%d] failure: %s\n",rv,client_fd,strerror(errno));
			close(client_fd);
			rv = -2;
		}
		close(client_fd);
	}
	close(listen_fd);
	return 0 ;
} 

