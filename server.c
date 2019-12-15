/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surjem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>


struct info{
  int client_fd;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
};

void erro(char *msg);

int max_number_of_clients;
int server_port;

void *new_client(void *info){
  struct info client_server = *(struct info*)info;
	int nread = 0,size=10,i;
	char buffer[10000],*token;
  char str_list[10000],str_fich_info[257],command[10000],file_name[100];
  int dados[size],server_fd_udp,client_fd=client_server.client_fd;
  struct dirent *info_dir;  // Pointer for directory entry
  DIR *directory;
  FILE *f;
  struct sockaddr_in server_addr=client_server.server_addr;
  struct sockaddr_in client_addr=client_server.client_addr;
  if((server_fd_udp= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0){
       printf("server udp socket not created\n");
       exit(-1);
  }
  if((bind(server_fd_udp,(struct sockaddr*)&server_addr,sizeof(server_addr))) < 0){
     printf("Failed to bind a socket\n");
     exit(-1);
  }

  //strcpy(message,"Insert command: ");
  //write(client_fd,message,sizeof(message));
  while (1) {
    nread =read(client_fd,command,sizeof(command));
	  command[nread] = '\0';
    printf("From client %d: %s\n",client_fd,command);
    if (strncmp(command,"DOWNLOAD",8)==0){
      //arranjar forma de enviar o ficheiro*************************************
      if(strncmp(command+9,"TCP",3)==0){
        if(strncmp(command+9+4,"NOR",3)==0){
          token=strtok(command," ");
          token=strtok(NULL," ");
          token=strtok(NULL," ");
          token=strtok(NULL," ");
          strcpy(file_name,"server_files/");
          strcat(file_name,token);
          if((f=fopen(file_name,"rb"))==NULL){
            strcpy(buffer,"The file request doesn't exist. Try LIST to obtain the available files.");
            write(client_fd,buffer,sizeof(buffer));
          }
          else{
            memset(buffer,'\0',sizeof(buffer));
            while(fread(buffer,1,sizeof(buffer),f)!=0){
              write(client_fd,buffer,sizeof(buffer));
              memset(buffer,'\0',sizeof(buffer));
            }
            strcpy(buffer,"EOF");
            write(client_fd,buffer,sizeof(buffer));
          }
        }
        else{
          //encriptar
        }
      }
      else if(strncmp(command+9,"UDP",3)==0){
        if(strncmp(command+9+4,"NOR",3)==0){
          token=strtok(command," ");
          token=strtok(NULL," ");
          token=strtok(NULL," ");
          token=strtok(NULL," ");
          strcpy(file_name,"server_files/");
          strcat(file_name,token);
          if((f=fopen(file_name,"rb"))==NULL){
            strcpy(buffer,"The file request doesn't exist. Try LIST to obtain the available files.");
            sendto(server_fd_udp,buffer,sizeof(buffer),0,(struct sockaddr *) &client_addr,sizeof(client_addr));
          }
          else{
            memset(buffer,'\0',sizeof(buffer));
            while(fread(buffer,1,1,f)!=0){
              printf("%s\n",buffer);
              sendto(server_fd_udp,buffer,sizeof(buffer),0,(struct sockaddr *) &client_addr,sizeof(client_addr));
              perror("sendto");
              memset(buffer,'\0',sizeof(buffer));
            }
            strcpy(buffer,"EOF");
            sendto(server_fd_udp,buffer,sizeof(buffer),0,(struct sockaddr *) &client_addr,sizeof(client_addr));
          }
        }
        else{
          //encriptar
        }
      }
    }
    else if (strcmp(command,"LIST")==0){
      memset(&str_list,'\0', sizeof(str_list));
      // opendir() returns a pointer of DIR type.
      directory = opendir("server_files");
      if (directory == NULL){  // opendir returns NULL if couldn't open directory
          printf("Couldn't open current directory;" );
      }
      while ((info_dir = readdir(directory)) != NULL){
        sprintf(str_fich_info,"%s\n", info_dir->d_name);
        if (strcmp(str_fich_info,".\n")!=0 && strcmp(str_fich_info,"..\n")!=0)
          strcat(str_list,str_fich_info);
      }
      write(client_fd,str_list,sizeof(str_list));

      closedir(directory);
    }
    else if (strcmp(command,"QUIT")==0){
      printf("The client %d will now close.\n",client_fd);
      fflush(stdout);
    	close(client_fd);
      pthread_exit(NULL);
    }
  }
}

int main(int argc, char *argv[]) {
  pthread_t tid;
  int fd, client, port;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;
  struct info* info_ptr;

  if(argc!=3){
    printf("./server <port> <max number of clients>\n");
    exit(-1);
  }
  server_port=atoi(argv[1]);
  max_number_of_clients=atoi(argv[2]);

  bzero((void *) &addr, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET,"127.0.0.2", &(addr.sin_addr));
  addr.sin_port = htons(server_port);

  if ( (fd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0)
	   erro("in socket function");
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	   erro("in bind function");
  if( listen(fd, max_number_of_clients) < 0)
	   erro("in listen function");
  client_addr_size = sizeof(client_addr);
  printf("Server ready to receive: \nAddress: %s Port: %d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
  while (1) {
    //clean finished child processes, avoiding zombies
    //must use WNOHANG or would block whenever a child process was working
    while(waitpid(-1,NULL,WNOHANG)>0);
    //wait for new connection
    client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
    //port=(int) ntohs(addr.sin_port);
    printf("Client %d connected\n",client);
    if (client > 0) {
      info_ptr=(struct info*)malloc(sizeof(struct info));
      info_ptr->client_fd=client;
      info_ptr->server_addr=addr;
      info_ptr->client_addr=client_addr;
      pthread_create(&tid, NULL,new_client, (void *)info_ptr);
    }
  }
  return 0;
}

void erro(char *msg){
	printf("Erro: %s\n", msg);
	exit(-1);
}
