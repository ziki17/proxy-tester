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
  
#define IP_PORT_SIZE 22
 
struct arg_struct {
    char * addr;
};
  
void * test_proxy (void * arguments)
{
  
    struct arg_struct *args = arguments;
      
    char *ip;
    char *port;
    char *ptr;
    ip = strtok_r(args->addr, ":", &ptr);
    port = strtok_r(NULL, ":", &ptr);

    struct sockaddr_in server;
    int sockfd;
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
		return 0;
	}

    char request[] = "HEAD http://www.msn.com HTTP/1.0\r\n\r\n";
  	
    if (send(sockfd, request, strlen(request), 0) < 0)
    {
		return 0;
    }

    char server_reply[500];
      
    if (recv(sockfd, server_reply, 500, 0) < 0)
    {
		return 0;
    }
  
    printf("%s \n \n",server_reply);
      
    close(sockfd);
      
    if (strstr(server_reply,"HTTP/1.1 200") || 
        strstr(server_reply,"HTTP/1.1 302") ||
        strstr(server_reply,"HTTP/1.0 200") || 
        strstr(server_reply,"HTTP/1.0 302"))
    {
        //FILE * fp;
        //char address[IP_PORT_SIZE];
        //fp = fopen(args->output_file,"a");
        //sprintf(address, "%s:%s", ip, port);
        //fputs(address,fp);
        //fclose(fp);
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
  
    pthread_t proxy_thread[amount_of_ips];
    struct arg_struct args[amount_of_ips];
  
    int b;
    for (b = 0; b < amount_of_ips; b++)
    {
        args[b].addr = (char *) &ip_array[b];
        
        if(pthread_create(&proxy_thread[b], NULL, test_proxy, &args[b])) {
            printf("Error creating");
        }
    }
      
    int c;
    for (c = 0; c < amount_of_ips; c++)
    {
        if(pthread_join(proxy_thread[c], NULL)) {
  
            printf("Error joining thread\n");
        }
    }
}
