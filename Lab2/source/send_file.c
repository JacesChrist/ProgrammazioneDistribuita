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
#include <sys/select.h>
#include <sys/time.h>

#include "receive_file.h"
#include "sockwrap.h"
#include "errlib.h"

int extern long_output;

int server_send_file_to_client(int);
void serverSendErr(int);

int server_send_file_to_client(int socket)
{
    int status_bar1, status_bar2, secTimer = 5;
    unsigned long int i, uliCount;
    char nome_file[50], *buf, buffer[50];
    FILE *file;
    uint32_t size, timestamp;
    struct stat st;

    //inizializzazione timer
    /*fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(socket, &set);
    timeout.tv_sec = secTimer;
    timeout.tv_sec = 0;
    if (long_output)
        printf("PASS timer inizializzato\n");*/

    //if (select(FD_SETSIZE, &set, NULL, NULL, &timeout))
    {
        //ricezione G E T ' '
        buf = malloc(4 * sizeof(char));
        if (read(socket, buf, 4) != 4)
        {
            free(buf);
            close(socket);
            return (-1);
        }
        if (strncmp(buf, "GET ", 4) != 0)
        {
            serverSendErr(socket);
            printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (1);
        }
        free(buf);
        if (long_output)
            printf("PASS GET' ' ricevuto\n");
    }
    /*else
    {
        printf("- TIMEOUT CONNESSIONE -");
        fflush(stdout);
    }*/

    i = 0;
    while (1)
    {
        //ricezione nome file
        read(socket, &nome_file[i], 1);
        if (nome_file[i] == 13)
        {
            nome_file[i] = '\0';
            if (long_output)
                printf("PASS nome file ricevuto\n");
            //ricezione (CR) LF
            read(socket, &nome_file[i + 1], 1);
            if (nome_file[i + 1] != 10)
            {
                serverSendErr(socket);
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (1);
            }
            if (long_output)
                printf("PASS CR LF ricevuti\n");
            break;
        }
        else
        {
            i++;
            if (i == 50)
            {
                serverSendErr(socket);
                printf("ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
                return (1);
            }
        }
    }

    printf("- RICHIESTO FILE '%s' -\n", nome_file);
    //apertura file
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        printf("- FILE INESISTENTE -\n");
        return (1);
    }
    if (long_output)
        printf("PASS file aperto\n");
    //invio + O K
    if (write(socket, "+OK\r\n", 5) != 5)
    {
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return (-1);
    }
    if (long_output)
        printf("PASS +OK CR LF inviato\n");
    //conteggio dimensione
    uliCount = 0;
    buf = malloc(sizeof(char));
    while (fscanf(file, "%c", buf) != EOF)
    {
        uliCount++;
    }
    fclose(file);
    free(buf);
    size = htonl(uliCount);
    write(socket, &size, 4);
    if (long_output)
        printf("PASS dimensione '%lu' Byte inviata\n", uliCount);
    printf("- INVIO IN CORSO ");
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    //scansione-invio file
    status_bar1 = uliCount / 10;
    status_bar2 = 0;
    buf = malloc(sizeof(char));
    for (i = 0; i < uliCount; i++)
    {
        fflush(stdout);
        fscanf(file, "%c", buf);
        if (write(socket, buf, 1) != 1)
        {
            serverSendErr(socket);
            printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
            return (-1);
        }
        //barra di aggiornamento
        if (i == status_bar2)
        {
            printf("#");
            status_bar2 += status_bar1;
        }
    }
    free(buf);
    printf(" -\n");
    if (long_output)
        printf("PASS file inviato\n");

    //invio timestamp
    stat(nome_file, &st);
    fclose(file);
    int ultima_modifica = st.st_mtime;
    timestamp = htonl(ultima_modifica);
    if (write(socket, &timestamp, 4) != 4)
    {
        printf("FATAL ERROR: line %d - file '%s'\n", __LINE__ - 3, __FILE__);
        return (-1);
    }
    if (long_output)
    {
        struct tm *timestamp_format;
        time_t ts = ultima_modifica;
        timestamp_format = localtime(&ts);
        strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timestamp_format);
        printf("PASS timestamp '%s' inviato\n", buffer);
    }

    return (1);
}

void serverSendErr(int socket_error)
{
    if (write(socket_error, "-ERR\r\n", 6) != 6)
    {
        printf("ERROR: line %d - file '%s'\n", __LINE__ - 2, __FILE__);
        return;
    }
    return;
}