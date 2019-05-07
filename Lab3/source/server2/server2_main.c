#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../send_file.h"
#include "../sockwrap.h"

void zombie_hunter(int);

char *prog_name;

int buffer_size = 1500;
int long_output = 1;
int fork_counter = 0;

int main(int argc, char *argv[]) //in *argv: nomeProgramma porta
{
	signal(SIGCHLD,zombie_hunter);

	int socket_passive, socket_son,i,pid_son;
	struct sockaddr_in server_address;
	socklen_t address_length;

	printf("* SERVER TCP *\n");

	//controllo parametri linea di comando
	if (argc != 2)
	{
		printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (atoi(argv[1]) < 1024)
	{
		printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (long_output)
		printf("PASS: parametri linea di comando\n");

	//creazione socket
	socket_passive = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_passive < 0)
		return (-1);
	if (long_output)
		printf("PASS: creazione socket\n");

	//definizione indirizzo
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (long_output)
		printf("PASS: definizione indirizzo\n");

	//binding socket
	if (bind(socket_passive, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (long_output)
		printf("PASS: binding socket\n");

	//inizio ascolto
	if (listen(socket_passive, 100) < 0)
	{
		printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
		return (-1);
	}
	if (long_output)
		printf("PASS: inizio loop ascolto\n");
	fork_counter = 0;
	printf("- IN ATTESA DI CONNESSIONE -\n");

	while (1)
	{
		//accettazione connesione
		address_length = sizeof(struct sockaddr_in);
		socket_son = accept(socket_passive, (struct sockaddr *)&server_address, &address_length);
		if (socket_son < 0)
		{
			printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
			return (-1);
		}
		
		//fork
		if((pid_son = fork()) < 0)
		{
			printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
			return (-1);
		}
		fork_counter++;
		if(pid_son>0) //padre
		{
			if(long_output)
				printf("PASS: fork effettuata %d\n",pid_son);
			if (close(socket_son) != 0)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (0);
            }			
		}
		else //figlio
		{	
			if (close(socket_passive) != 0)
            {
                if (long_output)
                    printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (0);
            }		
			printf("- CONNESSIONE STABILITA -\n");
			while ((i = server_send_file_to_client(socket_son)) > 0);
			if(i == -1)
				printf("- CONNESSIONE INTERROTTA -\n");
			if(long_output)
				printf("PASS: terminazione processo figlio %d\n",getpid());
			exit(0);
		}
	}
	return 0;
}

void zombie_hunter(int signal)
{
	if (signal != SIGCHLD) 
	{
		printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
	}
	else
	{
		wait(NULL);
		if(long_output)
				printf("PASS: processo zombie terminato\n");
		fork_counter--;
		if(fork_counter == 0)
			printf("- IN ATTESA DI CONNESSIONE -\n");
	}
	return;
}
