/*
 * SERVER
 * parametri:
 * - porta
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

char *prog_name;

void serverError(int);
int long_output = 1;

int main(int argc, char *argv[])
{
	socklen_t address_length;
	char c, car[1], nome_file[50], buf[4];
	int i, bar1, bar2, server_socket_figlio;
	unsigned long int uli[1];
	FILE *file;

	printf("* SERVER TCP *\n");

	//controllo parametri linea di comando
	if (argc != 2)
		serverError(1);
	if (atoi(argv[1]) <= 1024)
		serverError(5);
	if (long_output)
		printf("PASS parametri linea di comando\n");

	//creazione socket
	int server_socket_passivo = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket_passivo < 0)
		serverError(2);
	if (long_output)
		printf("PASS creazione socket\n");

	//definizione indirizzo
	struct sockaddr_in server_address; //definizione indirizzo
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (long_output)
		printf("PASS definizione indirizzo\n");

	//binding socket
	if (bind(server_socket_passivo, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
		serverError(3);
	if (long_output)
		printf("PASS binding socket\n");

	//inizio ascolto
	if (listen(server_socket_passivo, 100) < 0)
		serverError(4);
	if (long_output)
		printf("PASS inizio ascolto\n");

	while (1)
	{
		printf("- IN ATTESA DI CONNESSIONE -\n");

		//accettazione connesione
		address_length = sizeof(struct sockaddr_in);
		server_socket_figlio = accept(server_socket_passivo, (struct sockaddr *)&server_address, &address_length);
		if (server_socket_figlio < 0)
			serverError(5);
		printf("- CONNESSIONE STABILITA -\n");

		while (1)
		{
			//ricezione G E T ' '
			if (recv(server_socket_figlio, buf, 4, 0) <= 0)
			{
				printf("- CONNESSIONE CHIUSA -\n");
				break;
			}
			if (strcmp(buf, "GET ") == 0)
			{
				if (long_output)
					printf("PASS GET' ' ricevuto\n");
				i = 0;
				while (1)
				{
					//ricezione nome file
					recv(server_socket_figlio, &nome_file[i], 1, 0);
					if (nome_file[i] == '\0')
						break;
					else
						i++;
				}
				if (long_output)
					printf("PASS nome file ricevuto\n");
				//ricezione CR LF
				recv(server_socket_figlio, &c, 1, 0);
				if (c == 10)
				{
					recv(server_socket_figlio, &c, 1, 0);
					if (c == 13)
					{
						if (long_output)
							printf("PASS CR LF ricevuti\n");
						printf("- RICHIESTO FILE '%s' -\n", nome_file);
						//apertura file
						file = fopen(nome_file, "r");
						if (file == NULL)
							serverError(7);
						else
						{
							if (long_output)
								printf("PASS file aperto\n");
							//invio + O K
							send(server_socket_figlio, "+OK", 3, MSG_NOSIGNAL);
							if (long_output)
								printf("PASS + O K inviato\n");
							//conteggio dimensione
							uli[0] = 0;
							while (fscanf(file, "%c", &c) != EOF)
							{
								uli[0]++;
							}
							send(server_socket_figlio, uli, 4, MSG_NOSIGNAL);
							if (long_output)
								printf("PASS dimensione '%lu' Byte inviata\n", uli[0]);
							printf("- INVIO IN CORSO ");
							fclose(file);
							file = fopen(nome_file, "r");
							if (file == NULL)
								serverError(7);
							//scansione-invio file
							bar1 = uli[0] / 10;
							bar2 = 0;
							for (i = 0; i < uli[0]; i++)
							{
								fflush(stdout);
								fscanf(file, "%c", &car[0]);
								send(server_socket_figlio, car, 1, MSG_NOSIGNAL);
								//barra di aggiornamento
								if (i == bar2)
								{
									printf("#");
									bar2 += bar1;
								}
							}
							fclose(file);
							printf(" -\n");
							if (long_output)
								printf("PASS file inviato\n");
						}
					}
					else
						serverError(6);
				}
				else
					serverError(6);
			}
			else
				serverError(6);
		}
	}

	return 0;
}

void serverError(int codice_errore)
{
	switch (codice_errore)
	{
	case 1:
		printf("Errore: numero di parametri insufficiente\n");
		printf("Terminazione server\n");
		exit(-1);
	case 2:
		printf("Socket passivo non creato correttamente\n");
		printf("Terminazione server\n");
		exit(-1);
	case 3:
		printf("Bind socket-indirizzo fallito\n");
		printf("Terminazione server\n");
		exit(-1);
	case 4:
		printf("Socket figlio non creato correttamente\n");
		printf("Terminazione server\n");
		exit(-1);
	case 5:
		printf("'sudo' richiesto per la porta specificata\n");
		printf("Terminazione server\n");
		exit(-1);
	case 6:
		printf("Connessione tramite protocollo fallita\n");
		printf("Terminazione server\n");
		exit(-1);
	case 7:
		printf("File non aperto correttamente\n");
		printf("Terminazione server\n");
		exit(-1);

		printf("Errore non gestito\n");
		printf("Terminazione server\n");
		exit(-1);
	}
}
