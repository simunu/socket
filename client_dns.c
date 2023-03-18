/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  client_temp2.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/24/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/24/23 06:22:49"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

void print_usage(char *progname);
int get_temp(float *temp);

int main(int argc, char **argv)
{
	int     rv = -1;
	int     ch= 0 ;
	int     port = 0;
	int		time = 0;
	int     conn_fd =-1;
	float  temp = 0;
	char   str[1024] ;
	char   buf[1024];
	char   ipstr[1024];
	char   **hostip =0;
	char   *servip = 0;
	char   *hostname = 0;
	struct hostent  *servhost;
	struct  sockaddr_in servaddr;

	struct option  opts[] = {
		{"hostname",required_argument,NULL,'h'},
		{"port",required_argument,NULL,'p'},
		{"time",required_argument,NULL,'t'},
		{"Help",no_argument,NULL,'H'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc,argv,"h:p:t:H",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'h':
				hostname = optarg;
				break;
			case'p':
				port=atoi(optarg);
				break;
			case 't':
				time=atoi(optarg);
				break;
			case'H':
				print_usage(argv[0]);
				return 0;
		}
	}

	if((inet_aton(hostname,&servaddr.sin_addr)) == 0)
	{
		if((servhost = gethostbyname(hostname)) == NULL)
		{
			printf("Get hostname error: %s\n", strerror(errno));
			return -1;
		}
		switch(servhost -> h_addrtype)
		{
			case AF_INET6:
			case AF_INET:
				hostip = servhost -> h_addr_list;
				for (; *hostip != NULL; hostip++)
					printf("IP: %s\n",inet_ntop(servhost ->h_addrtype, servhost -> h_addr,ipstr,sizeof(ipstr)));
				servip = ipstr;
				break;
			default:
				printf("error address!\n");
				break;
		}
	}
	else
	{
		servip = hostname;
	}


	while(1)
	{
		conn_fd = socket(AF_INET,SOCK_STREAM,0);
		if(conn_fd < 0)
		{
			printf("create  socket failure:%s\n",strerror(errno));
			return  -1;
		}
		printf("create  socket[%d]  successfully\n",conn_fd);
		memset(&servaddr,0,sizeof(servaddr));
		servaddr.sin_family=AF_INET;
		servaddr.sin_port=htons(port);
		inet_aton(servip,&servaddr.sin_addr);
		if((connect(conn_fd,(struct sockaddr *)&servaddr,sizeof(servaddr)))<0)
		{
			printf("connect to server[%s:%d] failure:%s\n",servip,port, strerror(errno));
			return  -1;
		}
		printf("connect to  server[%s:%d]  successfully!\n",servip,port);

		rv= get_temp(&temp);
		if( rv  < 0)
		{
			printf("get temperature failure:%s\n",strerror(errno));
			return -1;
		}
		sprintf(buf,"%f\n",temp);

		rv = write(conn_fd,buf,sizeof(buf));
		if(rv<0 )
		{
			printf("write to server failure:%s\n",strerror(errno));
			goto cleanup;
		}
		memset(buf,0,sizeof(buf));
		rv= read(conn_fd,buf,sizeof(buf));
		if(rv<0)
		{
			printf("read data from server failure:%s\n",strerror(errno));
			goto cleanup;
		}
		else if(rv == 0)
		{
			printf("socket[%d] get disconnected\n",conn_fd);
			goto cleanup;
		}
		printf("the  temperature('c) is:%s\n", buf);
		sleep(time);
	}
cleanup:
	close(conn_fd);
	return 0;
}

void print_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("--h(--hostname): sepcify server hostname \n");
	printf("-p(--port): sepcify server port \n");
	printf("-t(--time):get the temperature time \n");
	printf("-H(--Help): print this help information \n");
}

int get_temp(float *temp)
{
	int     rv = 0;
	int     fd = -1; 
	int     found = 0;
	char    *ptr = 0;
	char    chip[20];
	char    buf[1024];
	char    ds_path[50];
	char    *w1_path = "/sys/bus/w1/devices";
	DIR     *dirp = 0;
	struct  dirent  *direntp;

	if((dirp = opendir(w1_path)) == NULL )
	{
		printf("opendir error: %s\n",strerror(errno));
		return -1;
	}

	while((direntp = readdir(dirp)) != NULL)
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strcpy(chip,direntp->d_name);
			found = 1;
			break;
		}
	}
	closedir(dirp);
	if(!found)
	{
		printf("can not find ds18b20 in %s\n",w1_path);
		return -1;
	}

	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);
	if((fd = open(ds_path,O_RDONLY)) < 0 )
	{
		printf("open %s error : %s\n",ds_path,strerror(errno));
		return -1;
	}
	if(read(fd,buf,sizeof(buf)) < 0)
	{
		printf("read %s error:%s\n",w1_path,strerror(errno));
		rv = -1;
		goto cleanup;
	}

	ptr = strstr(buf,"t=");
	if(!ptr)
	{
		printf("error:can not get temperature\n");
		rv = -1;
		goto cleanup;
	}
	ptr += 2;
	*temp = atof(ptr)/1000;
	snprintf(buf,sizeof(buf),"%f",*temp);

cleanup:
	close(fd);
	return rv;
}

