#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <errno.h>

int save_flag;
int losses;
struct connection* connection_list;
// A thread function
// A thread for each client request
struct connection{
  char client_ip[20],server_ip[20];
  int client_fd,server_port,client_port;
  struct connection* next;
};

struct info{
  int client_fd;
  struct sockaddr_in server_addr;
  struct sockaddr_in proxy_addr;
  struct sockaddr_in client_addr;
};

struct connection* create_list_connection(){
  struct connection *aux;
  aux = (struct connection*) malloc(sizeof(struct connection));
  if(aux != NULL){
      aux -> next = NULL;
  }
  return aux;
}
void add_connection_to_list(struct connection* head,char* client_ip,char*server_ip,int client_port,int server_port,int client_fd){
  struct connection* new_element=(struct connection*)malloc(sizeof(struct connection));
  new_element->client_fd=client_fd;
  new_element->client_port=client_port;
  new_element->server_port=server_port;
  strcpy(new_element->client_ip,client_ip);
  strcpy(new_element->server_ip,server_ip);
  new_element->next=head->next;
  head->next=new_element;
}
void remove_from_connection_list(struct connection *head,int client_fd){
  struct connection *ant=head,*current=head->next;
  do{
    if(current->client_fd==client_fd){
      ant->next=current->next;
      free(current);
      break;
    }
    current=current->next;
    ant=ant->next;
  }while(current!=NULL);
}
void print_connection_list(struct connection* head){
  struct connection* current=head;
  if(head->next==NULL){
    printf("There are no connections to the proxy at the moment.\n");
  }
  else{
  while(current->next!=NULL){
      current=current->next;
      printf("Client %d\n",current->client_fd);
      printf("\t->Origin ip: %s\n",current->client_ip);
      printf("\t->Origin port: %d\n",current->client_port);
      printf("\t->Destination ip: %s\n",current->server_ip);
      printf("\t->Destination port: %d\n",current->server_port);
    }
  }
}

void *new_client(void *info){
  struct info client_server = *(struct info*)info;
  struct sockaddr_in server_addr =client_server.server_addr;
  struct sockaddr_in proxy_addr =client_server.proxy_addr;
  struct sockaddr_in client_addr =client_server.client_addr;
  int server_fd,proxy_fd_udp,client_fd =client_server.client_fd;
  char buffer[10000],command[100],server_ip[100];
  int bytes=0,len_addr;

     //code to connect to main server via this proxy server
     // create a socket
     if((server_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
          printf("server tcp socket not created\n");
          exit(-1);
     }
     if((proxy_fd_udp= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
          printf("client udp socket not created\n");
          exit(-1);
     }
     if((bind(proxy_fd_udp, (struct sockaddr*)&proxy_addr,sizeof(proxy_addr))) < 0){
        printf("Failed to bind a socket client_fd_udp\n");
        exit(-1);
     }
     /*if((bind(server_fd_udp, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0){
        printf("Failed to bind a socket server_fd_udp\n");
        exit(-1);
     }*/
     //connect to main server from this proxy server
     if(connect(server_fd, (struct sockaddr*)&server_addr,(socklen_t)sizeof(server_addr))<0){
       perror("Error in connect");
       exit(-1);
     }
     //printf("server socket connected\n");
     while(1){
          bytes = read(client_fd,command,sizeof(command));
          if(bytes>0){
             // send data to main server
             write(server_fd,command, sizeof(command));
             printf("From client %d: %s\n",client_fd,command);
             fflush(stdout);
          }
          //receive response from server
          if(strncmp(command,"DOWNLOAD",8)==0){
            //enviar o ficheiro*************************************************
            if(strncmp(command+9,"TCP",3)==0){
              if(strncmp(command+9+4,"NOR",3)==0){
                do{
                  memset(buffer,'\0',sizeof(buffer));
                  read(server_fd,buffer, sizeof(buffer));
                  write(client_fd,buffer, sizeof(buffer));
                }while(strcmp(buffer,"EOF")!=0);              }
              else{
                //encriptar
              }
            }
            else if(strncmp(command+9,"UDP",3)==0){
              if(strncmp(command+9+4,"NOR",3)==0){
                memset(buffer,'\0',sizeof(buffer));
                len_addr=sizeof(server_addr);
                recvfrom(proxy_fd_udp,buffer,sizeof(buffer),0,(struct sockaddr *) &server_addr,(socklen_t *)&len_addr);
                printf("%s\n",buffer);
                sendto(proxy_fd_udp,buffer,sizeof(buffer), 0, (struct sockaddr *) &client_addr,sizeof(client_addr));
              }
              else{
                //encriptar
              }
            }

          }
          else if(strcmp(command,"LIST")==0){
            memset(&buffer,'\0', sizeof(buffer));
            memset(&command,'\0', sizeof(command));
            bytes=read(server_fd, buffer, sizeof(buffer));
            write(client_fd,buffer,sizeof(buffer));
          }
          else if(strcmp(command,"QUIT")==0){
            printf("The client thread %d will now close\n",client_fd);
            write(server_fd,buffer,strlen(buffer)+1);
            remove_from_connection_list(connection_list,client_fd);
            close(client_fd);
            pthread_exit(NULL);
          }
     }
}

void *proxy_read(){
  char command[200];
  char* token;
  printf("proxy_read created\n");
  while(1){
    fgets(command,200,stdin);
    command[strlen(command)-1]='\0';
    if(strncmp(command,"LOSSES",6)==0){
      token=strtok(command," ");
      token=strtok(NULL," ");
      losses=atoi(token);
      printf("losses:%d\n",losses);
    }
    else if(strcmp(command,"SHOW")==0){
      print_connection_list(connection_list);
    }
    else if(strcmp(command,"SAVE")==0){
      if(save_flag==0)
        save_flag=1;
      else
        save_flag=0;
    }
    else{
      printf("Wrong command\n");
    }
  }
}

// main entry point
int main(int argc,char *argv[]){
    pthread_t tid;
    char port[100],ip[100],buffer[100];
    char *hostname = argv[1];
    char proxy_port[100];
    //socket variables
    int proxy_fd=0, client_fd=0, proxy_fd_udp=0;
    struct sockaddr_in proxy_addr,server_addr,client_addr;
    struct info *info_ptr;
    connection_list=create_list_connection();
    if(argc!=2){
      printf("./proxy <portos>\n");
      exit(-1);
    }
     // accept arguments from terminal
     strcpy(proxy_port,argv[1]); // proxy port
     printf("proxy port is %s\n",proxy_port);
     // create a socket
     if((proxy_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
         printf("Failed to create socket\n");
         exit(-1);
     }
     if((proxy_fd_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
         printf("Failed to create socket udp\n");
         exit(-1);
     }
     printf("Proxy initialize\n");
     pthread_create(&tid,NULL,proxy_read,NULL);
     memset(&proxy_addr, 0, sizeof(proxy_addr));
     // set socket variables
     proxy_addr.sin_family = AF_INET;
     proxy_addr.sin_port = htons(atoi(proxy_port));
     inet_pton(AF_INET,"127.0.0.1", &(proxy_addr.sin_addr));
     // bind the socket
     if((bind(proxy_fd, (struct sockaddr*)&proxy_addr,sizeof(proxy_addr))) < 0){
        printf("Failed to bind a socket\n");
        exit(-1);
     }
     if((bind(proxy_fd_udp, (struct sockaddr*)&proxy_addr,sizeof(proxy_addr))) < 0){
        printf("Failed to bind a socket\n");
        exit(-1);
     }

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
                info_ptr->client_addr=client_addr;
                add_connection_to_list(connection_list,inet_ntoa(client_addr.sin_addr),inet_ntoa(server_addr.sin_addr),ntohs(client_addr.sin_port),ntohs(server_addr.sin_port),client_fd);
                pthread_create(&tid, NULL,new_client, (void *)info_ptr);
                sleep(1);
          }
     }
     return 0;
}
