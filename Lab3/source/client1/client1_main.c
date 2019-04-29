#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/socket.h>

#include "../receive_file.h"
#include "../sockwrap.h"

char *prog_name;

int buffer_size = 1000000;
int long_output = 1;

int main(int argc, char *argv[]) //in *argv: nomeProgramma indirizzo porta file(da 0 a n)
{
	int i, socket_client;
	char *buf;
	struct sockaddr_in client_address;

	printf("* CLIENT TCP *\n");

	//controllo argomenti linea di comando
	if (argc < 3)
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (atoi(argv[2]) < 1024)
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (argc == 3)
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (long_output)
		printf("PASS: parametri linea di comando\n");

	//creazione socket
	socket_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_client < 0)
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (long_output)
		printf("PASS: creazione socket\n");

	//definizione indirizzo
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(atoi(argv[2]));
	client_address.sin_addr.s_addr = inet_addr(argv[1]);
	if (long_output)
		printf("PASS: definizione indirizzo\n");

	//connessione dei socket
	if (connect(socket_client, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	printf("- CONNESSIONE STABILITA -\n");

	//protocollo di connessione

	for (i = 3; i < argc; i++)
	{
		if (client_receive_file_from_server(socket_client, argv[i]) < 0)
		{
			if (long_output)
				printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
			if (close(socket_client) != 0)
			{
				if (long_output)
					printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
				return (-1);
			}
			return (-1);
		}
	}
	if (close(socket_client) != 0)
	{
		if (long_output)
			printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
		return (-1);
	}
	printf("- CONNESSIONE CHIUSA -\n");

	return 0;
}
