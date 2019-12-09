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

#define BUF_SIZE	1024


void process_client(int fd);
void erro(char *msg);

int max_number_of_clients;
int server_port;

int main(int argc, char *argv[]) {
  int fd, client, port;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;

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
  printf("Socket created\n");
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	   erro("in bind function");
  printf("Bind successfull\n");
  if( listen(fd, 5) < 0)
	   erro("in listen function");
  printf("Listen successfull\n");
  client_addr_size = sizeof(client_addr);
  while (1) {
    //clean finished child processes, avoiding zombies
    //must use WNOHANG or would block whenever a child process was working
    while(waitpid(-1,NULL,WNOHANG)>0);
    //wait for new connection

    client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
    //port=(int) ntohs(addr.sin_port);
    printf("Client address: %s port: %d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
    if (client > 0) {
      if (fork() == 0) {
        close(fd);
        process_client(client);
        exit(0);
      }
    close(client);
    }
  }
  return 0;
}

void process_client(int client_fd)
{
	int nread = 0,size=10,i;
	char buffer[BUF_SIZE],*token,message[90];
  char str_list[10000],str_fich_info[257];
  int dados[size];
  int sum=0,flag=0;
  double ave=0;
  struct dirent *info_dir;  // Pointer for directory entry
  DIR *directory;

  memset(dados,-1,size*sizeof(int));

  strcpy(message,"Insert command: ");
  write(client_fd,message,sizeof(message));
  while (1) {
    nread = read(client_fd, buffer, BUF_SIZE-1);
	  buffer[nread] = '\0';
      if (strncmp(buffer,"DOWNLOAD",8)==0){
        //arranjar forma de enviar o ficheiro***********************************
      }
      else if (strcmp(buffer,"LIST")==0){
        // opendir() returns a pointer of DIR type.
        directory = opendir("server_files");
        if (directory == NULL){  // opendir returns NULL if couldn't open directory
            printf("Couldn't open current directory;" );
        }
        memset(&str_list,'\0', sizeof(str_list));
        while ((info_dir = readdir(directory)) != NULL){
                sprintf(str_fich_info,"%s\n", info_dir->d_name);
                strcat(str_list,str_fich_info);
        }
        printf("%s",str_list);


        closedir(directory);
      }
      else if (strcmp(buffer,"Quit")==0){
        printf("The client %d will now close.\n",client_fd);
        fflush(stdout);
      	close(client_fd);
        exit(0);
      }
  }


}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
