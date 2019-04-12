#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/socket.h>

#include "../receive_file.h"

#define buff_size 50

int long_output = 1;

void clientError(int);

int main(int argc, char *argv[]) //in *argv: nomeProgramma indirizzo porta file(da 0 a n)
{
	int i, j, bar1, bar2;
	unsigned long int uli[1], uliCount;
	char c, *buf, buffer[buff_size];

	printf("* CLIENT TCP *\n");

	//controllo argomenti linea di comando
	if (argc < 3)
		clientError(1);
	if (atoi(argv[2]) < 1024)
		clientError(5);
	if (argc == 3)
		clientError(2);
	if (long_output)
		printf("PASS parametri linea di comando\n");

	//creazione socket
	int client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < 0)
		clientError(3);
	if (long_output)
		printf("PASS creazione socket\n");

	//definizione indirizzo
	struct sockaddr_in client_address;
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(atoi(argv[2]));
	client_address.sin_addr.s_addr = inet_addr(argv[1]);
	if (long_output)
		printf("PASS definizione indirizzo\n");

	//connessione dei socket
	if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
		clientError(4);
	printf("- CONNESSIONE STABILITA -\n");

	//protocollo di connessione

	for (i = 0; i < (argc - 3); i++)
	{
		buf = malloc((7 + strlen(argv[3 + i])) * sizeof(char));
		sprintf(buf, "GET %s\r\n", argv[3 + i]);
		send(client_socket, buf, sizeof(buf), MSG_NOSIGNAL);
		free(buf);
		printf("- RICHIESTO FILE '%s' -\n", argv[3 + i]);
		buf = malloc(6*sizeof(char));
		recv(client_socket, buf, 6, 0); //PROBLEMA QUI
		if (strcmp(buf, "+OK\r\n") != 0)
			clientError(6);
		if (long_output)
			printf("PASS +OK ricevuto\n");
		if(client_receive_file_from_server(client_socket,argv[3+i])<0)
			return(-1);
		
	}
	printf("- CONNESSIONE CHIUSA -\n");

	return 0;
}

void clientError(int codErr)
{
	switch (codErr)
	{
	case 1:
		printf("Errore: numero di parametri insufficiente\n");
		printf("Terminazione client\n");
		exit(-1);
	case 2:
		printf("Nessun file specificato da trasferire\n");
		printf("Terminazione client\n");
		exit(1);
	case 3:
		printf("Socket non creato correttamente\n");
		printf("Terminazione client\n");
		exit(-1);
	case 4:
		printf("Connessione tra socket fallita\n");
		printf("Terminazione client\n");
		exit(-1);
	case 5:
		printf("'sudo' richiesto per la porta specificata\n");
		printf("Terminazione client\n");
		exit(-1);
	case 6:
		printf("Connessione tramite protocollo fallita\n");
		printf("Terminazione client\n");
		exit(-1);
	case 7:
		printf("File non aperto correttamente\n");
		printf("Terminazione client\n");
		exit(-1);

		printf("Errore non gestito\n");
		printf("Terminazione client\n");
		exit(-1);
	}
}