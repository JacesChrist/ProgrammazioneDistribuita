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
  socklen_t addrlen;
  char message[50];

  printf("QUESTO DOVREBBE ESSERE UN SERVER TCP\n");

  int mysocks = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP); //creazione socket
  //(famiglia IPV6 (internet),tipo STREAM (identifica TCP), protocollo TCP)
  if(mysocks == -1) error();
  printf("- socket passivo creato (id socket: %d)\n",mysocks);

  struct sockaddr_in saddr; //definizione indirizzo
  saddr.sin_family = AF_INET; //famiglia
  saddr.sin_port = htons(1500); //porta, <1024 devo essere su
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); //indirizzo sul quale aspettare (tutti i registrati sul calcolatore)
  printf("- struct indirizzo definito (porta %d, indirizzo %d)\n",saddr.sin_port,saddr.sin_addr.s_addr);

  i = bind(mysocks,(struct sockaddr *)&saddr,sizeof(saddr)); //binding socket (assegnazione indirizzo locale al socket)
  if(i == -1) error();
  printf("- binding effettuato con successo\n");

  i = listen(mysocks,100); //mette in ascolto socket
  if(i == -1) error();
  printf("- socket %d in ascolto \n",mysocks);

  addrlen = sizeof(struct sockaddr_in);
  printf("- socket %d in attesa \n",mysocks);
  int sock2 = accept(mysocks,(struct sockaddr *)&saddr,&addrlen);
  if(sock2 == -1) error();
  printf("- connessione stabilita (id socket: %d)\n",sock2);

  recv(sock2,message,sizeof(message),0);
  printf("- MESSAGGIO RICEVUTO:\n--- %s ---\n",message);

  return 0;
}

void error(void){
  printf("ERRORE\nTERMINAZIONE\n");
  exit(0);
}
