#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "receive_file.h"

int extern long_output;

int client_receive_file_from_server(int socket,char *file_name){
    char *buf,buffer;
    FILE *file;

    buf=malloc(4*sizeof(char));
    if(recv(socket, buf, 4, 0)!=4)
        return(-1);

	uint32_t size = ntohl((*(uint32_t *)buf));
	if (long_output)
			printf("PASS dimensione '%u' Byte ricevuta\n", size);
		/*file = fopen(argv[3 + i], "w");
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
				recv(socket, &c, 1, 0);
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
*/
    return(0);
};