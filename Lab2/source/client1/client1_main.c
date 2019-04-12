/*
 * TEMPLATE
 * parametri:
 * - indirizzo
 * - porta
 * - file (n)
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define buff_size 50

int long_output = 1;


void clientError(int);

int main(int argc, char *argv[]) //in *argv: nomeProgramma indirizzo porta file(da 0 a n)
{
	int i, j, bar1, bar2;
	unsigned long int uli[1],uliCount;
	char c, *buf,buffer[buff_size];
	FILE *file;

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
		/*send(client_socket, "GET ", 4, MSG_NOSIGNAL);
		if (long_output)
			printf("PASS GET' ' inviato\n");
		send(client_socket, argv[3 + i], strlen(argv[3 + i]) + 1, MSG_NOSIGNAL);
		if (long_output)
			printf("PASS nome file inviato\n");
		buf = malloc(sizeof(char));
		buf[0] = 13;
		send(client_socket,buf, 1, MSG_NOSIGNAL);
		buf[0] = 10;
		send(client_socket, buf, 1, MSG_NOSIGNAL);
		free(buf);
		if (long_output)
			printf("PASS CR LF inviato\n");*/
		sprintf(buffer,"GET %s\r\n",argv[3+i]);
		send(client_socket,buffer,sizeof(buffer), MSG_NOSIGNAL);
		printf("- RICHIESTO FILE '%s' -\n", argv[3 + i]);
		buf = malloc(3 * sizeof(char));
		recv(client_socket, buf, 3, 0);
		if (strcmp(buf, "+OK") == 0)
		{
			free(buf);
			if (long_output)
				printf("PASS +OK ricevuto\n");
			recv(client_socket, &uli[0], 4, 0);
			uint32_t size = ntohl((*(uint32_t *)uli));
			if (long_output)
				printf("PASS dimensione '%u' Byte ricevuta\n", size);
			file = fopen(argv[3 + i], "w");
			if (file == NULL)
				clientError(7);
			else
			{
				if (long_output)
					printf("PASS file creato\n");
				printf("- RICEZIONE IN CORSO ");
				bar1 = uli[0] / 10;
				for (j = 0; j < size; j++)
				{
					fflush(stdout);
					recv(client_socket, &c, 1, 0);
					fprintf(file, "%c", c);
					if (j == bar2)
					{
						printf("#");
						bar2 += bar1;
					}
				}
				printf(" -\n");
				fclose(file);
				if (long_output)
					printf("PASS file scritto\n");
				recv(client_socket, &uli[0], 4, 0);
				if (long_output)
					printf("PASS timestamp '%ld' ricevuto\n", uli[0]);
			}
		}
		else
			clientError(6);
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