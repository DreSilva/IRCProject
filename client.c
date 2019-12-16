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
#include <errno.h>
#include <time.h>
#include <sys/time.h>


void erro(char *msg);

int main(int argc, char *argv[]) {
  char server_address[100],proxy_address[100],command[100],received[10000],file_name_path[100];
  char *token,*file_name,*protocol;
  int fd_proxy,total_received_bytes=0,nread,len_addr,sd_udp;
  double total_download_time;
  struct sockaddr_in proxy_addr,server_addr,proxy_udp_addr;
  struct hostent *server_ptr,*proxy_ptr;
  struct timeval inicial_time,final_time;
  FILE *f;
  if (argc != 5) {
    	printf("./cliente <endereço Proxy> <endereço Servidor> <porto> <protocolo>\n");
    	exit(-1);
  }
  strcpy(proxy_address, argv[1]);
  strcpy(server_address, argv[2]);

  bzero((void *) &proxy_addr, sizeof(proxy_addr));
  proxy_addr.sin_family = AF_INET;
  inet_pton(AF_INET, proxy_address, &proxy_addr.sin_addr);
  proxy_addr.sin_port = htons((short) atoi(argv[3]));
  bzero((void *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET,server_address, &server_addr.sin_addr);
  server_addr.sin_port = htons((short) atoi(argv[3]));

  if(strcmp(argv[4],"TCP")==0){
    if((fd_proxy = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1)
  	 erro("Error creating socket");
    if(connect(fd_proxy,(struct sockaddr*)&proxy_addr,sizeof(proxy_addr)) < 0)
  	 erro("Error on connection to the proxy");

    printf("Sending server_addr to Proxy\n");
    write(fd_proxy,&server_addr,sizeof(server_addr));
    printf("Sending server_addr to Proxy successfull\n");
    while(1){
      //read(fd_proxy,&received,sizeof(received));
      //printf("%s\n",received);
      fgets(command,200,stdin);
      command[strlen(command)-1]='\0';
      if(strcmp(command,"LIST")==0){
        write(fd_proxy,command,sizeof(command));
        printf("LISTA DE FICHEIROS DISPONIVEIS\n-----------------------------------------\n");
        read(fd_proxy,received,sizeof(received));
        printf("%s",received);
        printf("-----------------------------------------\n");
      }
      else if(strncmp(command,"DOWNLOAD",8)==0){
        //**********************************************************************
        gettimeofday(&inicial_time, NULL);
        write(fd_proxy,command,sizeof(command));
        //rcv file and save file
        if(strncmp(command+9,"TCP",3)==0){
          if(strncmp(command+9+4,"NOR",3)==0){
            token=strtok(command," ");
            protocol=strtok(NULL," ");
            token=strtok(NULL," ");
            file_name=strtok(NULL," ");
            strcpy(file_name_path,"downloads/");
            strcat(file_name_path,file_name);
            remove(file_name_path);
            f=fopen(file_name_path,"ab");
            do{
              nread=read(fd_proxy,received,sizeof(received));
              if(strcmp(received,"The file requested doesn't exist. Try LIST to obtain the available files.")==0){
                printf("%s\n",received);
                remove(file_name_path);
                break;
              }
              if(strcmp(received,"EOF")==0){
                break;
              }
              else{
                if(strstr(file_name_path,".txt")!=NULL){
                  total_received_bytes+=strlen(received);
                  fwrite(received,1,strlen(received),f);
                }
                else{
                  total_received_bytes+=sizeof(received);
                  fwrite(received,1,sizeof(received),f);
                }
              }
            }while(strcmp(received,"EOF")!=0);
            fclose(f);
            gettimeofday(&final_time, NULL);
            total_download_time=final_time.tv_usec-inicial_time.tv_usec;
            printf("File name: %s\n",file_name);
            printf("Total received bytes: %d\n",total_received_bytes);
            printf("Protocol: %s\n",protocol);
            printf("Total download time: %lf us\n",total_download_time);
            total_received_bytes=0;
          }
          else if(strncmp(command+9+4,"ENC",3)==0){
          }
          else{
            printf("Wrong command\n");
          }
        }
        else if(strncmp(command+9,"UDP",3)==0){
          //********************************************************************
          bzero((void *) &proxy_udp_addr, sizeof(proxy_udp_addr));
          proxy_udp_addr.sin_family = AF_INET;
          inet_pton(AF_INET,proxy_address,&proxy_udp_addr.sin_addr) ;
          proxy_udp_addr.sin_port = htons((short)(atoi(argv[3])));
          if((sd_udp=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
              perror("Erro na criação do socket");
              exit(-1);
          }
          if(bind(sd_udp,(struct sockaddr*)&proxy_udp_addr, sizeof(proxy_udp_addr)) == -1){
              perror("Erro no bind");
              exit(-1);
          }

          if(strncmp(command+9+4,"NOR",3)==0){
            token=strtok(command," ");
            protocol=strtok(NULL," ");
            token=strtok(NULL," ");
            file_name=strtok(NULL," ");
            strcpy(file_name_path,"downloads/");
            strcat(file_name_path,file_name);
            remove(file_name_path);
            f=fopen(file_name_path,"ab");
            do{
              memset(received,'\0',sizeof(received));
              len_addr=sizeof(proxy_addr);
              nread=recvfrom(sd_udp,received,sizeof(received),0,(struct sockaddr *) &proxy_udp_addr, (socklen_t *)&len_addr);
              if(strcmp(received,"The file request doesn't exist. Try LIST to obtain the available files.")==0){
                remove(file_name_path);
                printf("%s\n",received);
                break;
              }
              if(strcmp(received,"EOF")==0){
                break;
              }
              else{
                if(strstr(file_name_path,".txt")!=NULL){
                  total_received_bytes+=strlen(received);
                  fwrite(received,1,strlen(received),f);
                }
                else{
                  total_received_bytes+=sizeof(received);
                  fwrite(received,1,nread,f);
                }
              }
            }while(strcmp(received,"EOF")!=0);
            fclose(f);
            gettimeofday(&final_time, NULL);
            total_download_time=final_time.tv_usec-inicial_time.tv_usec;
            printf("File name: %s\n",file_name);
            printf("Total received bytes: %d\n",total_received_bytes);
            printf("Protocol: %s\n",protocol);
            printf("Total download time: %lf us\n",total_download_time);
            total_received_bytes=0;
          }
          close(sd_udp);
        }
      }
      else if(strcmp(command,"QUIT")==0){
        write(fd_proxy,command,sizeof(command));
        printf("The client will now turn off\n");
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
