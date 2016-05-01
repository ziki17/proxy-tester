#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

int test_proxies (char * ips[], int amount_of_ips)
{

	int a;
	for(a = 0; a < amount_of_ips; a++)
	{

	   	char *ip;
	  	char *port;
		ip = strtok(ips[a], ":");
		port = strtok(NULL, ":");

		printf("%s::%s \n",ip,port);

		struct sockaddr_in server;
		int sockfd;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		server.sin_addr.s_addr = inet_addr(ip);
		server.sin_family = AF_INET;
		server.sin_port = htons(atoi(port));

		fcntl(sockfd, F_SETFL, O_NONBLOCK);

		struct timeval tv;
		fd_set writefds;

		tv.tv_sec = 10;
		tv.tv_usec = 500000;

		FD_ZERO(&writefds);
		FD_SET(sockfd, &writefds);


		if( connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			if ( errno != EINPROGRESS ) {
			return 0;
			}
		}
		
		if (select(sockfd + 1, NULL, &writefds, NULL, &tv) ==  1)
		{

			int so_error;
			socklen_t len = sizeof so_error;

			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

				if (so_error == 0) {
				printf("Connection open");
				fcntl(sockfd, F_SETFL, 0);
				}
		}
		else {
			continue;
		}
		
		char request[] = "HEAD http:/\/www.google.com HTTP/1.1\r\nHost: www.google.com\r\n\r\n";

		if (send(sockfd, request, strlen(request), 0) < 0)
		{
				printf("Error writing \n");
				return 1;
		}

		char server_reply[5000];

		if (recv(sockfd, server_reply, 5000, 0) < 1)
		{
			printf("Error receiving \n");
		}

		printf("%s \n",server_reply);

		free(ips[a]);

	}
}



int amount_ips (FILE * fp) {

	int ch, number_of_lines = 0;

	do 
	{
		ch = fgetc(fp);
		if(ch == '\n')
			number_of_lines++;
	} while (ch != EOF);

	return number_of_lines;
}




int main() {
	FILE * fp;
	char line[121];

	fp = fopen("file.txt","r");

	int amount_of_ips;
	amount_of_ips = amount_ips(fp);

	char * ips[amount_of_ips];

	fp = fopen("file.txt","r");

	int a;
	for (a = 0; a < amount_of_ips; a++)
	{
		fgets(line,120,fp);
		ips[a] = malloc(strlen(line) + 1);
		strcpy(ips[a], line);
	}

	fclose(fp);

	test_proxies(ips, amount_of_ips);
}
