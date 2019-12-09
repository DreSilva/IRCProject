/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

void erro(char *msg);

int main(int argc, char *argv[]) {
  char server_address[100],proxy_address[100],command[100],received[200];
  int fd_proxy;
  struct sockaddr_in proxy_addr,server_addr;
  struct hostent *server_ptr,*proxy_ptr;
  if (argc != 5) {
    	printf("./cliente <endereço Proxy> <endereço Servidor> <porto> <protocolo>\n");
    	exit(-1);
  }
  strcpy(proxy_address, argv[1]);
  strcpy(server_address, argv[2]);
  /*strcpy(server_address, argv[2]);
  if ((server_ptr = gethostbyname(server_address)) == 0)
    	erro("Was not able to obtain server address");
  strcpy(proxy_address, argv[1]);
  if ((proxy_ptr = gethostbyname(proxy_address)) == 0)
    	erro("Was not able to obtain proxy address");*/

  bzero((void *) &proxy_addr, sizeof(proxy_addr));
  proxy_addr.sin_family = AF_INET;
  inet_pton(AF_INET, proxy_address, &proxy_addr.sin_addr);
  proxy_addr.sin_port = htons((short) atoi(argv[3]));
  bzero((void *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET,server_address, &server_addr.sin_addr);
  server_addr.sin_port = htons((short) atoi(argv[3]));

  if(strcmp(argv[4],"TCP")==0){
    if((fd_proxy = socket(AF_INET,SOCK_STREAM,0)) == -1)
  	 erro("Error creating socket");
    if( connect(fd_proxy,(struct sockaddr*)&proxy_addr,sizeof(proxy_addr)) < 0)
  	 erro("Error on connection to the proxy");

    printf("Sending server_addr to Proxy\n");
    write(fd_proxy,&server_addr,sizeof(server_addr));
    printf("Sending server_addr to Proxy successfull\n");
    while(1){
      fgets(command,200,stdin);
      command[strlen(command)-1]='\0';
      if(strcmp(command,"LIST")==0){
        write(fd_proxy,command,sizeof(command));
        read(fd_proxy,received,sizeof(received));
        printf("%s",received);
      }
      else if(strncmp(command,"DOWNLOAD",8)==0){
        write(fd_proxy,command,sizeof(command));
        //rcv file and save file************************************************
      }
      else if(strcmp(command,"QUIT")==0){
        write(fd_proxy,command,sizeof(command));
        printf("O cliente vai desconetar\n");
        close(fd_proxy);
        exit(0);
      }
      else
        printf("Wrong command\n");
    }
  }
}

void erro(char *msg) {
	printf("Erro: %s\n", msg);
	exit(-1);
}
