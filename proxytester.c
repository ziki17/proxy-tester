#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define IP_PORT_SIZE 22

int test_proxies (char ips[][IP_PORT_SIZE], int amount_of_ips, char output_file[])
{
	int a;
	for(a = 0; a < amount_of_ips; a++)
	{
	   	char *ip;
	  	char *port;
		ip = strtok(ips[a], ":");
		port = strtok(NULL, ":");

		printf("%s:%s \n",ip,port);

		struct sockaddr_in server;
		int sockfd;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		server.sin_addr.s_addr = inet_addr(ip);
		server.sin_family = AF_INET;
		server.sin_port = htons(atoi(port));
				
		if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			//printf("Error connecting \n\n");
			continue;
		}
		
		char request[] = "HEAD http://www.msn.com HTTP/1.0\r\n\r\n";

		if (send(sockfd, request, strlen(request), 0) < 0)
		{
			//printf("Error writing \n\n");	
			continue;
		}

		char server_reply[500];
		
		if (recv(sockfd, server_reply, 500, 0) < 0)
		{
			//printf("Error receiving \n\n");
			continue;
		}

		//printf("%s \n \n",server_reply);
		
		close(sockfd);
		
		if (strstr(server_reply,"HTTP/1.1 200") || 
			strstr(server_reply,"HTTP/1.1 302") ||
			strstr(server_reply,"HTTP/1.0 200") || 
			strstr(server_reply,"HTTP/1.0 302"))
		{
			FILE * fp;
			char address[IP_PORT_SIZE];
			fp = fopen(output_file,"a");
			sprintf(address, "%s:%s", ip, port);
			fputs(address,fp);
			fclose(fp);
		}	
	}
}

int amount_ips (char file[]) {
	FILE * fp;
	int ch, number_of_lines = 0;

	fp = fopen(file,"r");

	do 
	{
		ch = fgetc(fp);
		if(ch == '\n')
			number_of_lines++;
	} while (ch != EOF);

	fclose(fp);

	return number_of_lines;
}

int main(int argc, char *argv[] ) {
		
	FILE * fp;
	int amount_of_ips;
	char line[IP_PORT_SIZE];	
	char file[100];
	strcpy(file,argv[1]);
	char output_file[100];
	strcpy(output_file,argv[2]);

	amount_of_ips = amount_ips(file);
	
	char ip_array[amount_of_ips][IP_PORT_SIZE];	

	fp = fopen(file,"r");

	int a;
	for (a = 0; a < amount_of_ips; a++)
	{
		fgets(line,IP_PORT_SIZE,fp);
		strcpy(ip_array[a], line);
	}

	fclose(fp);

	test_proxies(ip_array, amount_of_ips, output_file);
}
