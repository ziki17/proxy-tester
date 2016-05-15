#define _POSIX_SOURCE
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
#include <pthread.h>
#include <netdb.h>

typedef struct Args {
    char *addr;
    char *output_file;
    int type;
} Args;
 
char host_to_ip[100];

int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        perror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

void *test_proxy(void *arguments) {
    Args *args = arguments;
 
 	FILE *out = fopen(args->output_file, "a");

    char *ptr = NULL;
    char *ip = strtok_r(args->addr, ":", &ptr);
    char *port = strtok_r(NULL, "\n", &ptr);
 
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return NULL;
    }
  
    struct sockaddr_in server;
    memset(&server, 0, sizeof server);
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
     
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sockfd);
        perror("connect");
        return NULL;
    }
 	
 	char server_reply[500];
	memset(server_reply,0,sizeof(server_reply));
	ssize_t send_ret = 0;
	ssize_t recv_ret = 0;
	unsigned int dest_ip;
	unsigned int dest_port;
	
	switch (args->type)
	{
		case 1: // http
			;
			char http_request[] = "HEAD http://www.msn.com HTTP/1.0\r\n\r\n";
			
			send_ret = send(sockfd, http_request, strlen(http_request), 0);
			if (send_ret == -1) {
				close(sockfd);
				perror("send");
				return NULL;
			}
		 
			recv_ret = recv(sockfd, server_reply, sizeof server_reply, 0);
			server_reply[500] = '\0';

			if (recv_ret == -1) {
				close(sockfd);
				perror("recv");
				return NULL;
			}
						
			if (strstr(server_reply,"HTTP/1.1 200") || 
				strstr(server_reply,"HTTP/1.1 302") ||
				strstr(server_reply,"HTTP/1.0 200") || 
				strstr(server_reply,"HTTP/1.0 302"))
			{
				fprintf(out, "%s:%s\n", ip, port);  
			}	

			close(sockfd);
			
			break;
			
		case 2: // https
			;
			char https_request[] = "CONNECT www.msn.com:443 HTTP/1.0\r\n\r\n";
			
			send_ret = send(sockfd, https_request, strlen(https_request), 0);
			if (send_ret == -1) {
				close(sockfd);
				perror("send");
				return NULL;
			}

			recv_ret = recv(sockfd, server_reply, sizeof server_reply, 0);
			server_reply[500] = '\0';
			
			if (recv_ret == -1) {
				close(sockfd);
				perror("recv");
				return NULL;
			}

			if (strstr(server_reply,"HTTP/1.1 200") || 
				strstr(server_reply,"HTTP/1.1 302") ||
				strstr(server_reply,"HTTP/1.0 200") || 
				strstr(server_reply,"HTTP/1.0 302"))
			{
				fprintf(out, "%s:%s\n", ip, port);  
			}	
				
			close(sockfd);
			
			break;
		
		case 3: // socks 4
			;
			char socks4_buff[100];
			memset(socks4_buff,0,sizeof(socks4_buff));
			
			socks4_buff[0]=4;
			socks4_buff[1]=1;
			
			dest_ip = inet_addr(host_to_ip);
			dest_port = htons(80);

			memcpy(&socks4_buff[2],&dest_port,2); // dest port	  
			memcpy(&socks4_buff[4],&dest_ip,4); // dest host    

			send_ret = send(sockfd, socks4_buff, 9, 0);
			if (send_ret == -1) {
				close(sockfd);
				perror("send");
				return NULL;
			}
						
			recv_ret = recv(sockfd, server_reply, sizeof server_reply, 0);
			if (recv_ret == -1) {
				close(sockfd);
				perror("recv");
				return NULL;
			}
			
			if (server_reply[1] == 0x5A)
			{				
				fprintf(out, "%s:%s\n", ip, port);  
			}				
			
			close(sockfd);

			break;
		
		case 4: // socks 5
			;			
			char socks5_buff[100];
			memset(socks5_buff,0,sizeof(socks5_buff));
			
			socks5_buff[0]=5;
			socks5_buff[1]=1;
			socks5_buff[2]=0;
		  
			send_ret = send(sockfd, socks5_buff, 3, 0);
			if (send_ret == -1) {
				close(sockfd);
				perror("send");
				return NULL;
			}
						
			recv_ret = recv(sockfd, server_reply, sizeof server_reply, 0);
			if (recv_ret == -1) {
				close(sockfd);
				perror("recv");
				return NULL;
			}
						
			if (server_reply[0] == 0x05 && server_reply[1] == 0x00)
			{				
				memset(socks5_buff,0,sizeof(socks5_buff));
				
				socks5_buff[0]=5; // socks version
				socks5_buff[1]=1;	//nomber of methods
				socks5_buff[2]=0;   // no auth method
				socks5_buff[3]=1;
    				
				dest_ip = inet_addr(host_to_ip);
				dest_port = htons(80);
				
				memcpy(&socks5_buff[4],&dest_ip,4); // dest host    
				memcpy(&socks5_buff[8],&dest_port,2); // dest port

				send_ret = send(sockfd, socks5_buff, 10, 0);
				if (send_ret == -1) {
					close(sockfd);
					perror("send");
					return NULL;
				}

				recv_ret = recv(sockfd, server_reply, 10, 0);
				if (recv_ret == -1) {
					close(sockfd);
					perror("recv");
					return NULL;
				}
				
				if (server_reply[0] == 0x05 && server_reply[1] == 0x00) {
					fprintf(out, "%s:%s\n", ip, port);  
				}
				
				close(sockfd);
			}
	}
	
	close(sockfd);
	fclose(out);
}
   
int count_ips(const char *file) {
    int ch, number_of_lines = 0;
    FILE *fp = fopen(file,"r");
    do {
        ch = fgetc(fp);
        if(ch == '\n')
            number_of_lines++;
    } while (ch != EOF);
    fclose(fp);
    return number_of_lines;
}
   
int main(int argc, char *argv[] ) {
    char line[100];
    const char *file = argv[1];
    const char *output_file = argv[2];
 
    int num_ips = count_ips(file);
    char ip_array[num_ips][sizeof line];
   
    FILE *fp = fopen(file,"r");
    int i;
    for (i = 0; i < num_ips; i++) {
        fgets(line, sizeof line, fp);
        strcpy(ip_array[i], line);
    }
    fclose(fp);
	
	hostname_to_ip("www.msn.com", host_to_ip);
    
    pthread_t proxy_thread[num_ips];
    Args args[num_ips];
    for (i = 0; i < num_ips; i++) {
        args[i].addr = ip_array[i];
        args[i].type = atoi(argv[3]);
        args[i].output_file = output_file;
        if (pthread_create(&proxy_thread[i], NULL, test_proxy, &args[i])) {
            perror("pthread_create");
		}
    }
       
    for (i = 0; i < num_ips; i++) {
        if (pthread_join(proxy_thread[i], NULL)) {
            perror("pthread_join");
		}
	}
		
    return 0;
}
