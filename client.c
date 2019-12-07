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

void erro(char *msg);

int main(int argc, char *argv[]) {
  char endServer[100],command[100],received[200];
  int fd_proxy;
  struct sockaddr_in proxy_addr;
  struct hostent *hostPtr;

  if (argc != 5) {
    	printf("cliente <endereço Proxy> <endereço Servidor> <porto> <protocolo>\n");
    	exit(-1);
  }
  strcpy(endServer, argv[2]);
  if ((hostPtr = gethostbyname(endServer)) == 0)
    	erro("Nao consegui obter endereço");

  bzero((void *) &proxy_addr, sizeof(proxy_addr));
  proxy_addr.sin_family = AF_INET;
  proxy_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  proxy_addr.sin_port = htons((short) atoi(argv[3]));

  if(strcmp(argv[5],"TCP")==0){
    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
  	 erro("socket");
    if( connect(fd_proxy,(struct sockaddr *)&proxy_addr,sizeof (proxy_addr)) < 0)
  	 erro("Connect");
    while(1){
      fgets(command,200,stdin);
      command[strlen(command)-1]='\0';
      if(strcmp(command,"LIST")==0){
        write(fd_proxy,command,sizeof(command));
        read(fd_proxy,received,sizeof(received));
        printf("%s",received);
      }
      else if(strncmp(command,"DOWNLOAD",8)==0){
        write(fd,command,sizeof(command));
        //rcv file and save file************************************************
      }
      else if(strcmp()==0){
        write(fd,command,sizeof(command));
        printf("O cliente vai desconetar\n", );
        close(fd);
        exit(0);
      }
      else
        printf("Wrong command\n", );
    }
  }
}

void erro(char *msg) {
	printf("Erro: %s\n", msg);
	exit(-1);
}
