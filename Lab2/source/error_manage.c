#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "error_manage.h"

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
	case 8:
		printf("Nome file sfora 50 caratteri\n");
		printf("Terminazione server\n");
		exit(-1);
	case 9:
		printf("Funzione stat() fallita\n");
		printf("Terminazione server\n");
		exit(-1);
	case 10:
		printf("Messaggio di errore non mandato correttamente\n");
		printf("Terminazione server\n");
		exit(-1);

		printf("Errore non gestito\n");
		printf("Terminazione server\n");
		exit(-1);
	}
}

void serverSendErr(int server_socket_error)
{
	char c[1];
	int i;

	i = send(server_socket_error, "-ERR", 4, MSG_NOSIGNAL);
	c[0] = 10;
	i += send(server_socket_error, c, 1, MSG_NOSIGNAL);
	c[0] = 13;
	i += send(server_socket_error, c, 1, MSG_NOSIGNAL);
	if (i != 6)
		serverError(10);
	return;
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