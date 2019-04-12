#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/socket.h>

#include "../receive_file.h"
//#include "../error_manage.h"
#include "../sockwrap.h"

#define buff_size 50

char *prog_name;

int long_output = 1;

int main(int argc, char *argv[]) //in *argv: nomeProgramma indirizzo porta file(da 0 a n)
{
	int i;
	char *buf;

	printf("* CLIENT TCP *\n");

	//controllo argomenti linea di comando
	if (argc < 3)
		return(-1);
	if (atoi(argv[2]) < 1024)
		return(-1);
	if (argc == 3)
		return(-1);
	if (long_output)
		printf("PASS parametri linea di comando\n");

	//creazione socket
	int client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < 0)
		return(-1);
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
		return(-1);
	printf("- CONNESSIONE STABILITA -\n");

	//protocollo di connessione

	for (i = 3; i < argc; i++)
	{
		//buf = malloc(3 * sizeof(char));
		//sprintf(buf, "GET %s\r\n", argv[3]);
		if (write(client_socket, "GET ", 4) != 4)
			return (-1);
		if (long_output)
			printf("PASS GET' ' inviato\n");
		if (write(client_socket, argv[i], strlen(argv[i])) != strlen(argv[i]))
			return (-1);
		if (long_output)
			printf("PASS nome file inviato\n");
		if (write(client_socket, "\r\n", 2) != 2)
			return (-1);
		if (long_output)
			printf("PASS CR LF inviato\n");
		//free(buf);
		printf("- RICHIESTO FILE '%s' -\n", argv[3]);
		buf = malloc(5 * sizeof(char));
		if (read(client_socket, buf, 5) != 5)
			return (-1); //PROBLEMA QUI
		if (strcmp(buf, "+OK\r\n") != 0)
			return(-1);
		free(buf);
		if (long_output)
			printf("PASS +OK ricevuto\n");
		if (client_receive_file_from_server(client_socket, argv[3]) < 0)
			return (-1);
	}
	printf("- CONNESSIONE CHIUSA -\n");

	return 0;
}
