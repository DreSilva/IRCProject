#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include<unistd.h>
#include<netdb.h> //hostent
#include<arpa/inet.h>
int hostname_to_ip(char * , char *);
// A structure to maintain client fd, and server ip address and port address
// client will establish connection to server using given IP and port
struct serverInfo
{
     int client_fd;
     char ip[100];
     char port[100];
     char protocol[3];
};
// A thread function
// A thread for each client request
void *new_client(void *server)
{
  struct serverInfo *info = (struct serverInfo *)server;
  char buffer[65535];
  int bytes=0;
     printf("client:%d\n",info->client_fd);
     //code to connect to main server via this proxy server
     int server_fd=0;
     struct sockaddr_in server_sd;
     // create a socket
     if(server_fd = socket(AF_INET, SOCK_STREAM, 0) < 0){
          printf("server socket not created\n");
          exit(-1);
        }
     printf("server socket created\n");
     memset(&server_sd, 0, sizeof(server_sd));
     // set socket variables
     server_sd.sin_family = AF_INET;
     server_sd.sin_port = htons(atoi(info->port));
     server_sd.sin_addr.s_addr = inet_addr(info->ip);
     //connect to main server from this proxy server
     if((connect(server_fd, (struct sockaddr *)&server_sd, sizeof(server_sd)))<0)
          printf("server connection not established");
     printf("server socket connected\n");
     while(1)
     {
          //receive data from client
          memset(&buffer, '\0', sizeof(buffer));
          bytes = read(info->client_fd, buffer, sizeof(buffer));
          if(bytes>0){
             // send data to main server
             write(server_fd, buffer, sizeof(buffer));
             //printf("client fd is : %d\n",c_fd);
             printf("From client :\n");
             fputs(buffer,stdout);
             fflush(stdout);
          }
          //receive response from server
          if(strncmp(buffer,"DOWNLOAD",8)==0){
            memset(&buffer, '\0', sizeof(buffer));
            //enviar o ficheiro*************************************************
          }
          else if((strcmp(buffer,"LIST")==0){
            memset(&buffer, '\0', sizeof(buffer));
            bytes = read(server_fd, buffer, sizeof(buffer));
            write(info->client_fd,buffer,sizeof(buffer))
          }
          else if((strcmp(buffer,"QUIT")==0){
            printf("The client thread %d will now close",client_fd);
            close(info->client_fd);
            pthread_exit(NULL);
          }
     };
  return NULL;
}
// main entry point
int main(int argc,char *argv[])
{
    pthread_t tid;
    char port[100],ip[100];
    char *hostname = argv[1];
    char proxy_port[100]=argv[1];
    //socket variables
    int proxy_fd=0, client_fd=0;
    struct sockaddr_in proxy_sd;
    if(argc!=1){
      printf("ircproxy <portos>\n", );
      exit(-1);
    }
     // accept arguments from terminal
     //strcpy(ip,argv[1]); // server ip
     //strcpy(port,argv[2]);  // server port
     strcpy(proxy_port,argv[1]); // proxy port
     //hostname_to_ip(hostname , ip);
     printf("proxy port is %s",proxy_port);
     printf("\n");
     // create a socket
     if((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
         printf("\nFailed to create socket");
     printf("Proxy initialize\n");
     memset(&proxy_sd, 0, sizeof(proxy_sd));
     // set socket variables
     proxy_sd.sin_family = AF_INET;
     proxy_sd.sin_port = htons(atoi(proxy_port));
     proxy_sd.sin_addr.s_addr = INADDR_ANY;
     // bind the socket
     if((bind(proxy_fd, (struct sockaddr*)&proxy_sd,sizeof(proxy_sd))) < 0)
        printf("Failed to bind a socket");

     // start listening to the port for new connections
     if((listen(proxy_fd, SOMAXCONN)) < 0)
        printf("Failed to listen");
     //accept all client connections continuously
     while(1)
     {
          client_fd = accept(proxy_fd, (struct sockaddr*)NULL ,NULL);
          printf("client no. %d connected\n",client_fd);
          if(client_fd > 0)
          {
                //multithreading variables
                struct serverInfo *info = malloc(sizeof(struct serverInfo));
                info->client_fd = client_fd;
                strcpy(info->ip,"127.0.0.1");
                strcpy(info->port,"9000");
                pthread_create(&tid, NULL, new_client, (void *)info);
                sleep(1);
          }
     }
     return 0;
}
