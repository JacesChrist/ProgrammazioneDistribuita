#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h> /*per timeouts*/
#include <netinet/in.h> /*per Internet*/
#include <unistd.h> /*per close*/

void error(void);

int main(int argc,char **argv){
  int i;
  char message[20] = "LiMortiOsciSquaiati";

  if(argc != 3) error();
  printf("QUESTO DOVREBBE ESSERE UN CLIENT TCP\nIndirizzo: [ %s ]\nPorta: [ %d ]\n",argv[1],atoi(argv[2]));

  int mysockc = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP); //creazione socket
  //(famiglia IPV6 (internet),tipo STREAM (identifica TCP), protocollo TCP)
  if(mysockc == -1) error();
  printf("- socket creato (id socket: %d)\n",mysockc);

  struct sockaddr_in saddr; //definizione indirizzo
  saddr.sin_family = AF_INET; //famiglia
  //saddr.sin_port = atoi(argv[2]); ?????
  saddr.sin_port = htons(1500); //porta, <1024 devo essere su
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); //indirizzo sul quale inviare
  //saddr.sin_addr.s_addr = *argv[1]; ?????
  printf("- struct indirizzo definito (porta %d, indirizzo %d)\n",saddr.sin_port,saddr.sin_addr.s_addr);

  i = connect(mysockc,(struct sockaddr *)&saddr,sizeof(saddr)); //connessione ad una destinazione remota
  if(i == -1) error();
  printf("- connessione stabilita\n");

  if((send(mysockc,message,sizeof(message),MSG_NOSIGNAL)) != sizeof(message)) error(); //invio messaggio
  printf("- messaggio inviato { %s }\n",message);

  return 0;
}

void error(void){
  printf("ERRORE\nTERMINAZIONE\n");
  exit(0);
}
