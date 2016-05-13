/* Thanks to algorism for improvements. */

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
 
typedef struct Args {
    char *addr;
    char outfile[100];
} Args;
 
void *test_proxy(void *arguments) {
    Args *args = arguments;
 
    char *ptr = NULL;
    char *ip = strtok_r(args->addr, ":", &ptr);
    char *port = strtok_r(NULL, "\n", &ptr);
 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
 
    char request[] = "HEAD http://www.msn.com HTTP/1.0\r\n\r\n";
    ssize_t send_ret = send(sockfd, request, strlen(request), 0);
    if (send_ret == -1) {
        close(sockfd);
        perror("send");
        return NULL;
    }
 
    FILE *fout = fopen(args->outfile, "w");
 
    char server_reply[500];
    ssize_t n = 0;
    while ((n = recv(sockfd, server_reply, sizeof server_reply - 1, 0)) > 0) {
        server_reply[n] = '\0';
        fprintf(fout, "%s", server_reply);
    }
    fputc('\n', fout);
    if (n == -1)
        perror("recv");
 
    fclose(fout);
    close(sockfd);
    return NULL;
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
   
    pthread_t proxy_thread[num_ips];
    Args args[num_ips];
    for (i = 0; i < num_ips; i++) {
        args[i].addr = ip_array[i];
        sprintf(args[i].outfile, "%s_%d", output_file, i);
        if (pthread_create(&proxy_thread[i], NULL, test_proxy, &args[i]))
            perror("pthread_create");
    }
       
    for (i = 0; i < num_ips; i++)
        if (pthread_join(proxy_thread[i], NULL))
            perror("pthread_join");
 
    return 0;
}
