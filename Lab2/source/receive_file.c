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
#include "error_manage.h"
#include "sockwrap.h"
#include "errlib.h"

int extern long_output;

int client_receive_file_from_server(int socket, char *file_name)
{
    char *buf, buffer[50];
    int i, status_bar1, status_bar2;
    FILE *file;
    uint32_t size,timestamp;

    buf = malloc(4 * sizeof(char));
    if (read(socket, buf, 4) != 4)
        return (-1);
     size = ntohl((*(uint32_t *)buf));
    free(buf);
    if (long_output)
        printf("PASS dimensione '%u' Byte ricevuta\n", size);
    file = fopen(file_name, "w");
    if (file == NULL)
        clientError(7);
    else
    {
        if (long_output)
            printf("PASS file creato\n");
        printf("- RICEZIONE IN CORSO ");
        status_bar1 = size / 10;
        status_bar2 = 0;
        buf = malloc(1 * sizeof(char));
        for (i = 0; i < size; i++)
        {
            fflush(stdout);
            read(socket, buf, 1);
            fprintf(file, "%s", buf);
            if (i == status_bar2)
            {
                printf("#");
                status_bar2 += status_bar1;
            }
        }
        printf(" -\n");
        fclose(file);
        free(buf);
        if (long_output)
            printf("PASS file scritto\n");

        buf = malloc(4 * sizeof(char));

        if (read(socket, buf, 4) != 4)
            return -1;
        if (long_output)
            printf("PASS timestamp ricevuto\n");
        timestamp = ntohl((*(uint32_t *)buf));
        time_t ts = timestamp;
        struct tm *timestamp_format;
        timestamp_format = localtime(&ts);
        strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
        printf("- ULTIMA MODIFICA FILE: %s\n", buffer);
    }

    return (0);
};