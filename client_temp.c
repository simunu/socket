/*********************************************************************************
 *      Copyright:  (C) 2022 Zhu Lijun
 *                  All rights reserved.
 *
 *       Filename:  client_temp1.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(10/26/22)
 *         Author:  Zhu Lijun <3262465970@qq.com>
 *      ChangeLog:  1, Release initial version on "10/26/22 03:46:12"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>


//#define SERVER_IP   "127.0.0.1"
//#define SERVER_PORT  8889
#define MSG_STR   "Get the temperature successfully!"


/*  void printf_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("--i(--ipaddr): sepcify server IP address \n");
	printf("-p(--port): sepcify server port \n");
	printf("-h(--help): print this help information \n");
}
*/

int ds18b20_get_temperature(float *temp)
{
	char    *w1_path = "/sys/bus/w1/devices";
	char    ds_path[50];
    char    chip[20];
	char    buf[1024];
	DIR     *dirp;
    struct  dirent  *direntp;
	int     ds18b20_fd = -1;
	char    *ptr;
	int     found = 0;
	int     ds18b20_rv = 0;
	float   t = 0;

	 
	if((dirp = opendir(w1_path)) == NULL )
	{
		printf("opendir error: %s\n",strerror(errno));
		return -2;
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
        return -3;
	}
	
	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);
	  
	if((ds18b20_fd = open(ds_path,O_RDONLY)) < 0 )
	{
		printf("open %s error : %s\n",ds_path,strerror(errno));
		return -4;
	}
	   
	if(read(ds18b20_fd,buf,sizeof(buf)) < 0)
	{
		printf("read %s error:%s\n",ds_path,strerror(errno));
		ds18b20_rv = -5;
		goto cleanup;
	}
	
	
	ptr = strstr(buf,"t=");
	if(!ptr)
	{
        printf("error:can not get temperature\n");
        ds18b20_rv = -7;
		goto cleanup;
	}
	ptr += 2;
	*temp = atof(ptr)/1000;
	snprintf(buf,sizeof(buf),"%f",*temp);
//	strcpy(temp,buf);


cleanup:
	close(ds18b20_fd);
	return ds18b20_rv;


}

int main(int argc, char ** argv)
{
	int		conn_fd =-1;
	int 	rv = -1;
	char 	buf[1024];
	struct 	sockaddr_in serv_addr;
	int		T = 0;
	char	*serverip;
	int		port;
	float  temp;
	char   str[64];
//	int 	ch;
//	int		index;
/*	struct option  opts[] = {
		{"ipaddr",required_argument,NULL,'i'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc,argv,"i:p:h",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'i':
				serverip = optarg;
				break;
			case'p':
				port = atoi(optarg);
				break;
			case'h':
				printf_usage(argv[0]);
				return 0;
		}
	}
*/


	
		while(argc < 3)
		{
		printf("please input %s [serverip] [port]\n",argv[0]);
		return -1;
		}
		serverip = argv[1];
		port = atoi(argv [2]);

		while(1)
		{
		if((conn_fd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		{
			printf("socket failure:%s\n",strerror(errno));
			return -1;
		}
		printf("creat socket[%d] successfully\n",conn_fd);
										 	 
		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);
		inet_aton(serverip,&serv_addr.sin_addr);

		if((connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0)
		{
			printf("connect socket failure:%s\n",strerror(errno));
			return -2;
		}
	
	
		rv = ds18b20_get_temperature(&temp);
		if( rv < 0 )
		{
			printf("get temperature failure:%s\n",strerror(errno));
			return -1;
		}
		sprintf(buf,"%f\n",temp);
		rv = write(conn_fd,buf,sizeof(buf));

		if(rv < 0 )
		{
			printf("write failure:%s\n",strerror(errno));
			goto cleanup;
		}
		memset(buf,0,sizeof(buf));
		rv = read(conn_fd,buf,sizeof(buf));
		if( rv < 0)
		{
			printf("read failure:%s\n",strerror(errno));
			goto cleanup;
		}
		printf("read data from server:\nthe temperature('c) is:%s",buf);
		sleep(10);
	}
cleanup:
		close(conn_fd);
		return 0;
}

