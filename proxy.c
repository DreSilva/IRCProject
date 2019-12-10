#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include<unistd.h>
#include<netdb.h> //hostent
#include<arpa/inet.h>
#include<errno.h>

// A thread function
// A thread for each client request
struct info{
  int client_fd;
  struct sockaddr_in server_addr;
};
void *new_client(void *info){
  struct info client_server = *(struct info*)info;
  struct sockaddr_in server_addr =client_server.server_addr;
  int server_fd,client_fd=client_server.client_fd;
  char buffer[10000],server_ip[100];
  int bytes=0;

     //code to connect to main server via this proxy server
     // create a socket
     if((server_fd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0){
          printf("server socket not created\n");
          exit(-1);
     }
     //connect to main server from this proxy server
     if(connect(server_fd, (struct sockaddr*)&server_addr,(socklen_t)sizeof(server_addr))<0){
       perror("Error in connect");
       exit(-1);
     }
     //printf("server socket connected\n");
     while(1)
     {
          //receive data from client
          /*memset(&buffer, '\0', sizeof(buffer));
          read(server_fd,&buffer,sizeof(buffer));
          write(client_fd,&buffer,sizeof(buffer));
          memset(&buffer, '\0', sizeof(buffer));*/
          bytes = read(client_fd, buffer, sizeof(buffer));
          if(bytes>0){
             // send data to main server
             write(server_fd, buffer, sizeof(buffer));
             printf("From client:");
             fputs(buffer,stdout);
             fflush(stdout);
          }
          //receive response from server
          if(strncmp(buffer,"DOWNLOAD",8)==0){
            memset(&buffer, '\0', sizeof(buffer));
            //enviar o ficheiro*************************************************
          }
          else if(strcmp(buffer,"LIST")==0){
            memset(&buffer,'\0', sizeof(buffer));
            bytes=read(server_fd, buffer, sizeof(buffer));
            buffer[bytes]='\0';
            write(client_fd,buffer,sizeof(buffer));
          }
          else if(strcmp(buffer,"QUIT")==0){
            printf("The client thread %d will now close",client_fd);
            write(server_fd,buffer,strlen(buffer)+1);
            close(client_fd);
            pthread_exit(NULL);
          }
     };
  return NULL;
}
// main entry point
int main(int argc,char *argv[]){
    pthread_t tid;
    char port[100],ip[100],buffer[100];
    char *hostname = argv[1];
    char proxy_port[100];
    //socket variables
    int proxy_fd=0, client_fd=0;
    struct sockaddr_in proxy_addr,server_addr,client_addr;
    struct info *info_ptr;
    if(argc!=2){
      printf("./proxy <portos>\n");
      exit(-1);
    }
     // accept arguments from terminal
     strcpy(proxy_port,argv[1]); // proxy port
     printf("proxy port is %s\n",proxy_port);
     // create a socket
     if((proxy_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
         printf("\nFailed to create socket");
         exit(-1);
     }
     printf("Proxy initialize\n");
     memset(&proxy_addr, 0, sizeof(proxy_addr));
     // set socket variables
     proxy_addr.sin_family = AF_INET;
     proxy_addr.sin_port = htons(atoi(proxy_port));
     inet_pton(AF_INET,"127.0.0.1", &(proxy_addr.sin_addr));
     // bind the socket
     if((bind(proxy_fd, (struct sockaddr*)&proxy_addr,sizeof(proxy_addr))) < 0)
        printf("Failed to bind a socket");

     // start listening to the port for new connections
     if((listen(proxy_fd, SOMAXCONN)) < 0)
        printf("Failed to listen");
     //accept all client connections continuously
     while(1)
     {
          int client_addr_size=sizeof(client_addr);
          client_fd = accept(proxy_fd, (struct sockaddr*)&client_addr,(socklen_t *)&client_addr_size);
          printf("client no. %d connected\n",client_fd);
          if(client_fd > 0){
                //multithreading variables
                read(client_fd,&server_addr,sizeof(server_addr));
                printf("Server info received\nServer address: %s port: %d\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));
                info_ptr=(struct info*)malloc(sizeof(struct info));
                info_ptr->server_addr=server_addr;
                info_ptr->client_fd=client_fd;
                pthread_create(&tid, NULL, new_client, (void *)info_ptr);
                sleep(1);
          }
     }
     return 0;
}
